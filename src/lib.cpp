//===-- lib.cpp - TEK Injector API implementation -------------------------===//
//
// Copyright (c) 2025 Nuclearist <nuclearist@teknology-hub.com>
// Part of tek-injector, under the GNU General Public License v3.0 or later
// See https://github.com/teknology-hub/tek-injector/blob/main/COPYING for
//    license information.
// SPDX-License-Identifier: GPL-3.0-or-later
//
//===----------------------------------------------------------------------===//
///
/// @file
///  Implementation of TEK Injector functions.
///
//===----------------------------------------------------------------------===//
#include "tek-injector.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>

namespace {

/// The first message sent over the pipe to TEK Game Runtime.
struct pipe_header {
  /// Settings loading method.
  tek_gr_load_type type;
  /// Size of the remaining data that will be sent to the pipe, in bytes. When
  ///    @ref type is @ref TEK_GR_LOAD_TYPE_file_path, the data is path to the
  ///    file, or "tek-gr-settings.json" in current directory if the size is
  ///    zero. When @ref type is @ref TEK_GR_LOAD_TYPE_pipe, the data is actual
  ///    settings JSON content.
  std::uint32_t size;
};

/// RAII wrapper for most OS handles.
class [[gnu::visibility("internal")]] unique_handle {
protected:
  HANDLE value;

public:
  constexpr unique_handle() noexcept : value{nullptr} {}
  constexpr unique_handle(HANDLE handle) noexcept : value{handle} {}
  ~unique_handle() noexcept { close(); }
  constexpr PHANDLE operator&() noexcept { return &value; }
  constexpr operator bool() const noexcept { return static_cast<bool>(value); }
  constexpr operator HANDLE() const noexcept { return value; }
  void close() noexcept {
    if (value) {
      CloseHandle(value);
      value = nullptr;
    }
  }
};

/// RAII wrapper for file handles.
class [[gnu::visibility("internal")]] unique_file {
  HANDLE value;

public:
  constexpr unique_file() noexcept : value{INVALID_HANDLE_VALUE} {}
  ~unique_file() noexcept { close(); }
  constexpr void operator=(HANDLE handle) noexcept { value = handle; }
  constexpr operator bool() const noexcept {
    return value != INVALID_HANDLE_VALUE;
  }
  constexpr operator HANDLE() const noexcept { return value; }
  void close() noexcept {
    if (value != INVALID_HANDLE_VALUE) {
      CloseHandle(value);
      value = INVALID_HANDLE_VALUE;
    }
  }
};

/// RAII wrapper for process handles that terminates on failure.
struct [[gnu::visibility("internal")]] unique_process : public unique_handle {
  bool success;
  constexpr unique_process(HANDLE handle) noexcept
      : unique_handle{handle}, success{false} {}
  ~unique_process() noexcept {
    if (!success) {
      TerminateProcess(value, 0);
    }
  }
};

} // namespace

extern "C" void tek_inj_run_game(tek_inj_game_args *args) {
  const auto token{GetCurrentProcessToken()};
  bool elevated;
  // Check if current process is elevated
  {
    TOKEN_ELEVATION_TYPE elevation_type;
    DWORD ret_size;
    if (!GetTokenInformation(token, TokenElevationType, &elevation_type,
                             sizeof elevation_type, &ret_size)) {
      args->result = TEK_INJ_RES_get_token_info;
      args->win32_error = GetLastError();
      return;
    }
    elevated = elevation_type == TokenElevationTypeFull;
  }
  // Build command line
  std::wstring command_line{args->exe_path};
  if (command_line.contains(L' ')) {
    for (auto pos{command_line.find(L'"')}; pos != std::string::npos;
         pos = command_line.find(L'"', pos)) {
      command_line.replace(pos, 1, L"\\\"");
      pos += 2;
    }
    command_line.insert(command_line.begin(), L'"');
    command_line.push_back(L'"');
  }
  for (auto &&arg :
       std::span{args->argv, static_cast<std::size_t>(args->argc)} |
           std::views::transform([](auto ptr) { return std::wstring{ptr}; })) {
    if (arg.empty()) {
      arg = L"\"\"";
    } else if (arg.contains(L' ')) {
      for (auto pos{arg.find(L'"')}; pos != std::string::npos;
           pos = arg.find(L'"', pos)) {
        arg.replace(pos, 1, L"\\\"");
        pos += 2;
      }
      arg.append(std::ranges::find_last_if_not(
                     arg, [](auto ch) { return ch == L'\\'; })
                     .advance(1)
                     .size(),
                 L'\\');
      arg.insert(arg.begin(), L'"');
      arg.push_back(L'"');
    }
    command_line.push_back(L' ');
    command_line.append(arg);
  }
  // Create suspended game process
  const DWORD create_flags = (args->flags & TEK_INJ_FLAG_high_proc_prio)
                                 ? CREATE_SUSPENDED | HIGH_PRIORITY_CLASS
                                 : CREATE_SUSPENDED;
  STARTUPINFOW startup_info{};
  startup_info.cb = sizeof startup_info;
  PROCESS_INFORMATION proc_info;
  if (elevated && !(args->flags & TEK_INJ_FLAG_run_as_admin)) {
    // Copy current process token and set its integrity level to medium first,
    //    so game process runs without elevation
    unique_handle proc_token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &proc_token)) {
      args->result = TEK_INJ_RES_open_token;
      args->win32_error = GetLastError();
      return;
    }
    unique_handle mil_token;
    if (!DuplicateTokenEx(proc_token,
                          TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY |
                              TOKEN_ADJUST_DEFAULT,
                          nullptr, SecurityImpersonation, TokenPrimary,
                          &mil_token)) {
      args->result = TEK_INJ_RES_duplicate_token;
      args->win32_error = GetLastError();
      return;
    }
    proc_token.close();
    SID mil_sid{.Revision = 1,
                .SubAuthorityCount = 1,
                .IdentifierAuthority = SECURITY_MANDATORY_LABEL_AUTHORITY,
                .SubAuthority = {SECURITY_MANDATORY_MEDIUM_RID}};
    TOKEN_MANDATORY_LABEL label{
        .Label = {.Sid = &mil_sid, .Attributes = SE_GROUP_INTEGRITY}};
    if (!SetTokenInformation(mil_token, TokenIntegrityLevel, &label,
                             sizeof label)) {
      args->result = TEK_INJ_RES_set_token_info;
      args->win32_error = GetLastError();
      return;
    }
    if (!CreateProcessAsUserW(mil_token, args->exe_path, command_line.data(),
                              nullptr, nullptr, FALSE, create_flags, nullptr,
                              args->current_dir, &startup_info, &proc_info)) {
      args->result = TEK_INJ_RES_create_process;
      args->win32_error = GetLastError();
      return;
    }
  } else { // if (elevated && !(args->flags & TEK_INJ_FLAGS_run_as_admin))
    if (!CreateProcessW(args->exe_path, command_line.data(), nullptr, nullptr,
                        FALSE, create_flags, nullptr, args->current_dir,
                        &startup_info, &proc_info)) {
      args->result = TEK_INJ_RES_create_process;
      args->win32_error = GetLastError();
      return;
    }
  } // if (elevated && !(args->flags & TEK_INJ_FLAGS_run_as_admin)) else
  command_line = {};
  unique_process process{proc_info.hProcess};
  const unique_handle thread{proc_info.hThread};
  // Allocate memory for DLL path
  const std::wstring_view dll_path{args->dll_path};
  const auto dll_path_size{(dll_path.length() + 1) *
                           sizeof(decltype(dll_path)::value_type)};
  auto deleter{[&process](LPVOID addr) {
    VirtualFreeEx(process, addr, 0, MEM_RELEASE);
  }};
  std::unique_ptr<VOID, decltype(deleter)> mem{
      VirtualAllocEx(process, nullptr, dll_path_size, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE),
      deleter};
  if (!mem) {
    args->result = TEK_INJ_RES_mem_alloc;
    args->win32_error = GetLastError();
    return;
  }
  // Write DLL path to the allocated page
  if (!WriteProcessMemory(process, mem.get(), dll_path.data(), dll_path_size,
                          nullptr)) {
    args->result = TEK_INJ_RES_mem_write;
    args->win32_error = GetLastError();
    return;
  }
  // Create pipe for communication with TEK Game Runtime
  unique_file pipe;
  const DWORD buf_size = sizeof(pipe_header) + args->data_size;
  if (elevated && !(args->flags & TEK_INJ_FLAG_run_as_admin)) {
    // Initialize security descriptor with DACL that allows only current user
    //    and SACL that allows access for medium integrity level
    DWORD ret_size;
    if (!GetTokenInformation(token, TokenUser, nullptr, 0, &ret_size)) {
      const auto err{GetLastError()};
      if (err != ERROR_INSUFFICIENT_BUFFER) {
        args->result = TEK_INJ_RES_get_token_info;
        args->win32_error = err;
        return;
      }
    }
    auto token_user_buf{std::make_unique_for_overwrite<char[]>(ret_size)};
    if (!GetTokenInformation(token, TokenUser, token_user_buf.get(), ret_size,
                             &ret_size)) {
      args->result = TEK_INJ_RES_get_token_info;
      args->win32_error = GetLastError();
      return;
    }
    const auto user_sid{
        reinterpret_cast<const TOKEN_USER *>(token_user_buf.get())->User.Sid};
    const auto dacl_size{sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                         GetLengthSid(user_sid) - sizeof(DWORD)};
    const auto dacl_buf{std::make_unique_for_overwrite<char[]>(dacl_size)};
    const auto dacl{reinterpret_cast<PACL>(dacl_buf.get())};
    if (!InitializeAcl(dacl, dacl_size, ACL_REVISION)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    if (!AddAccessAllowedAce(dacl, ACL_REVISION, GENERIC_ALL, user_sid)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    token_user_buf.reset();
    SID mil_sid{.Revision = 1,
                .SubAuthorityCount = 1,
                .IdentifierAuthority = SECURITY_MANDATORY_LABEL_AUTHORITY,
                .SubAuthority = {SECURITY_MANDATORY_MEDIUM_RID}};
    std::array<char, sizeof(ACL) + sizeof(SYSTEM_MANDATORY_LABEL_ACE) +
                         sizeof mil_sid - sizeof(DWORD)>
        sacl_buf;
    const auto sacl{reinterpret_cast<PACL>(sacl_buf.data())};
    if (!InitializeAcl(sacl, sacl_buf.size(), ACL_REVISION)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    if (!AddMandatoryAce(sacl, ACL_REVISION, 0,
                         SYSTEM_MANDATORY_LABEL_NO_READ_UP, &mil_sid)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    SECURITY_DESCRIPTOR desc;
    if (!InitializeSecurityDescriptor(&desc, SECURITY_DESCRIPTOR_REVISION)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    if (!SetSecurityDescriptorDacl(&desc, TRUE, dacl, FALSE)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    if (!SetSecurityDescriptorSacl(&desc, TRUE, sacl, FALSE)) {
      args->result = TEK_INJ_RES_sec_desc;
      args->win32_error = GetLastError();
      return;
    }
    SECURITY_ATTRIBUTES attrs{.nLength = sizeof attrs,
                              .lpSecurityDescriptor = &desc,
                              .bInheritHandle = FALSE};
    pipe =
        CreateNamedPipeW(L"\\\\.\\pipe\\tek-game-runtime",
                         PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                             PIPE_REJECT_REMOTE_CLIENTS,
                         1, buf_size, buf_size, 0, &attrs);
  } else { // if (elevated && !(args->flags & TEK_INJ_FLAGS_run_as_admin))
    pipe =
        CreateNamedPipeW(L"\\\\.\\pipe\\tek-game-runtime",
                         PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                             PIPE_REJECT_REMOTE_CLIENTS,
                         1, buf_size, buf_size, 0, nullptr);
  } // if (elevated && !(args->flags & TEK_INJ_FLAGS_run_as_admin)) else
  if (!pipe) {
    args->result = TEK_INJ_RES_create_pipe;
    args->win32_error = GetLastError();
    return;
  }
  // Create the thread for injecting the DLL
  unique_handle inj_thread{
      CreateRemoteThread(process, nullptr, 0,
                         reinterpret_cast<LPTHREAD_START_ROUTINE>(
                             reinterpret_cast<void *>(LoadLibraryW)),
                         mem.get(), 0, nullptr)};
  if (!inj_thread) {
    args->result = TEK_INJ_RES_create_thread;
    args->win32_error = GetLastError();
    return;
  }
  // Wait for TEK Game Runtime to connect to the pipe
  if (!ConnectNamedPipe(pipe, nullptr)) {
    const auto err{GetLastError()};
    if (err != ERROR_PIPE_CONNECTED) {
      args->result = TEK_INJ_RES_connect_pipe;
      args->win32_error = err;
      return;
    }
  }
  // Write header and data to the pipe, then close the pipe as it's no longer
  //    needed
  const pipe_header hdr{.type = args->type, .size = args->data_size};
  DWORD bytes_written;
  if (!WriteFile(pipe, &hdr, sizeof hdr, &bytes_written, nullptr)) {
    args->result = TEK_INJ_RES_write_pipe;
    args->win32_error = GetLastError();
    return;
  }
  auto remaining{args->data_size};
  for (auto next{args->data}; remaining;) {
    if (!WriteFile(pipe, next, remaining, &bytes_written, nullptr)) {
      args->result = TEK_INJ_RES_write_pipe;
      args->win32_error = GetLastError();
      return;
    }
    next += bytes_written;
    remaining -= bytes_written;
  }
  pipe.close();
  // Wait for injection thread to finish and close it
  switch (WaitForSingleObject(inj_thread, 3000)) {
  case WAIT_OBJECT_0:
    break;
  case WAIT_TIMEOUT:
    SetLastError(ERROR_TIMEOUT);
    [[fallthrough]];
  default:
    args->result = TEK_INJ_RES_write_pipe;
    args->win32_error = GetLastError();
    TerminateThread(inj_thread, 0);
    return;
  }
  // Check injection thread exit code
  DWORD exit_code;
  if (GetExitCodeThread(inj_thread, &exit_code)) {
    args->win32_error = 0;
  } else {
    exit_code = 0;
    args->win32_error = GetLastError();
  }
  inj_thread.close();
  mem.reset();
  if (!exit_code) {
    args->result = TEK_INJ_RES_dll_load;
    return;
  }
  // Resume game's main thread execution
  if (ResumeThread(thread) == static_cast<DWORD>(-1)) {
    args->result = TEK_INJ_RES_resume_thread;
    args->win32_error = GetLastError();
    return;
  }
  process.success = true;
  args->result = TEK_INJ_RES_ok;
}
