//===-- lib.cpp - TEK Injector executable implementation ------------------===//
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
///  Implementation of TEK Injector executable utility.
///
//===----------------------------------------------------------------------===//
#include "tek-injector.h"

#include <comdef.h>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <memory>
#include <shobjidl.h>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

/// RAII wrapper for COM library.
struct [[gnu::visibility("internal")]] com_ctx {
  HRESULT hr;
  com_ctx() noexcept
      : hr{CoInitializeEx(nullptr,
                          COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)} {}
  ~com_ctx() noexcept {
    if (SUCCEEDED(hr)) {
      CoUninitialize();
    }
  }
};

/// RAII wrapper COM objects.
template <typename T>
  requires std::derived_from<T, IUnknown>
class [[gnu::visibility("internal")]] com_ptr {
  T *_Nullable ptr;

public:
  constexpr com_ptr() noexcept : ptr{nullptr} {}
  ~com_ptr() { release(); }
  constexpr T **operator&() noexcept { return &ptr; }
  constexpr T *operator->() noexcept { return ptr; }
  constexpr operator T *() const noexcept { return ptr; }
  void release() {
    if (ptr) {
      ptr->Release();
      ptr = nullptr;
    }
  }
};

static constexpr tek_inj_flag &operator|=(tek_inj_flag &left,
                                          tek_inj_flag right) noexcept {
  return left = static_cast<tek_inj_flag>(static_cast<int>(left) |
                                          static_cast<int>(right));
}

static inline void display_error(LPCWSTR msg) {
  MessageBoxW(nullptr, msg, L"TEK Injector", MB_OK | MB_ICONERROR);
}

static inline std::unique_ptr<WCHAR, decltype(&LocalFree)>
get_os_err_msg(DWORD err) {
  LPWSTR msg{nullptr};
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                 nullptr, err, 0, reinterpret_cast<LPWSTR>(&msg), 0, nullptr);
  return {msg, LocalFree};
}

} // namespace

int wmain(int argc, wchar_t *argv[]) {
  std::wstring exe_path;
  std::wstring current_dir;
  std::wstring dll_path{L"libtek-game-runtime.dll"};
  std::vector<LPCWSTR> game_argv;
  tek_inj_flag flags = TEK_INJ_FLAG_none;
  std::string settings_path;
  // Scan command line
  const std::span arg_span{argv, static_cast<std::size_t>(argc)};
  for (auto it{arg_span.begin() + 1}; it < arg_span.end(); ++it) {
    const std::wstring_view view{*it};
    if (view == L"--ti-exe-path") {
      if (++it < arg_span.end()) {
        exe_path = *it;
      }
    } else if (view == L"--ti-current-dir") {
      if (++it < arg_span.end()) {
        current_dir = *it;
      }
    } else if (view == L"--ti-dll-path") {
      if (++it < arg_span.end()) {
        dll_path = *it;
      }
    } else if (view == L"--ti-high-priority") {
      flags |= TEK_INJ_FLAG_high_proc_prio;
    } else if (view == L"--ti-run-as-admin") {
      flags |= TEK_INJ_FLAG_run_as_admin;
    } else if (view == L"--ti-settings-path") {
      if (++it < arg_span.end()) {
        const std::wstring_view path_view{*it};
        settings_path.resize(WideCharToMultiByte(CP_UTF8, 0, path_view.data(),
                                                 path_view.length(), nullptr, 0,
                                                 nullptr, nullptr));
        WideCharToMultiByte(CP_UTF8, 0, path_view.data(), path_view.length(),
                            settings_path.data(), settings_path.size(), nullptr,
                            nullptr);
      }
    } else {
      game_argv.emplace_back(*it);
    }
  } // for (auto it{arg_span.begin()}; it < arg_span.end(); ++it)
  if (exe_path.empty()) {
    // Select executable path via a dialog
    com_ctx ctx;
    if (FAILED(ctx.hr)) {
      display_error(std::format(L"CoInitializeEx failed: {}",
                                _com_error{ctx.hr}.ErrorMessage())
                        .data());
      return EXIT_FAILURE;
    }
    com_ptr<IFileOpenDialog> dialog;
    auto hr{CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                             CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))};
    if (FAILED(hr)) {
      display_error(std::format(L"Failed to create file dialog: {}",
                                _com_error{hr}.ErrorMessage())
                        .data());
      return EXIT_FAILURE;
    }
    hr = dialog->Show(nullptr);
    switch (hr) {
    case S_OK:
      break;
    case HRESULT_FROM_WIN32(ERROR_CANCELLED):
      return EXIT_SUCCESS;
    default:
      display_error(std::format(L"Failed to open file dialog: {}",
                                _com_error{hr}.ErrorMessage())
                        .data());
      return EXIT_FAILURE;
    }
    com_ptr<IShellItem> item;
    hr = dialog->GetResult(&item);
    if (FAILED(hr)) {
      display_error(std::format(L"Failed to get file dialog result: {}",
                                _com_error{hr}.ErrorMessage())
                        .data());
      return EXIT_FAILURE;
    }
    LPWSTR path;
    hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
    if (FAILED(hr)) {
      display_error(std::format(L"Failed to get selected file path: {}",
                                _com_error{hr}.ErrorMessage())
                        .data());
      return EXIT_FAILURE;
    }
    exe_path = path;
    CoTaskMemFree(path);
  } else { // if (exe_path.empty())
    // Convert the path from the command line to absolute
    const auto abs_path_len{
        GetFullPathNameW(exe_path.data(), 0, nullptr, nullptr)};
    if (!abs_path_len) {
      display_error(std::format(L"Failed to get full executable path: {}",
                                get_os_err_msg(GetLastError()).get())
                        .data());
      return EXIT_FAILURE;
    }
    std::wstring abs_path(abs_path_len - 1, L'\0');
    GetFullPathNameW(exe_path.data(), abs_path_len, abs_path.data(), nullptr);
    exe_path = std::move(abs_path);
  } // if (exe_path.empty()) else
  if (current_dir.empty()) {
    // Set executable's parent directory as current
    current_dir = std::filesystem::path{exe_path}.parent_path();
  }
  tek_inj_game_args args{.exe_path = exe_path.data(),
                         .current_dir = current_dir.data(),
                         .dll_path = dll_path.data(),
                         .type = TEK_GR_LOAD_TYPE_file_path,
                         .argc = static_cast<int>(game_argv.size()),
                         .argv = game_argv.data(),
                         .flags = flags,
                         .data_size =
                             static_cast<std::uint32_t>(settings_path.length()),
                         .data = settings_path.data(),
                         .result = TEK_INJ_RES_ok,
                         .win32_error = 0};
  tek_inj_run_game(&args);
  if (args.result == TEK_INJ_RES_ok) {
    return EXIT_SUCCESS;
  }
  std::wstring msg;
  switch (args.result) {
  case TEK_INJ_RES_get_token_info:
    msg = L"Failed to get process token information";
    break;
  case TEK_INJ_RES_open_token:
    msg = L"Failed to open current process token";
    break;
  case TEK_INJ_RES_duplicate_token:
    msg = L"Failed to duplicate process token";
    break;
  case TEK_INJ_RES_set_token_info:
    msg = L"Failed to set token information";
    break;
  case TEK_INJ_RES_create_process:
    msg = L"Failed to create game process";
    break;
  case TEK_INJ_RES_mem_alloc:
    msg = L"Failed to allocate memory in game process";
    break;
  case TEK_INJ_RES_mem_write:
    msg = L"Failed to write to game process memory";
    break;
  case TEK_INJ_RES_sec_desc:
    msg = L"Failed to setup security descriptor for the pipe";
    break;
  case TEK_INJ_RES_create_pipe:
    msg = L"Failed to create pipe";
    break;
  case TEK_INJ_RES_create_thread:
    msg = L"Failed to create injection thread";
    break;
  case TEK_INJ_RES_connect_pipe:
    msg = L"Failed to connect pipe";
    break;
  case TEK_INJ_RES_write_pipe:
    msg = L"Failed to write to pipe";
    break;
  case TEK_INJ_RES_thread_wait:
    msg = L"Failed to wait for injection thread to finish";
    break;
  case TEK_INJ_RES_dll_load:
    msg = L"TEK Game Runtime failed to load";
    break;
  case TEK_INJ_RES_resume_thread:
    msg = L"Failed to resume game's main thread";
    break;
  default:
    msg = std::format(L"Unknown result code {}", static_cast<int>(args.result));
    break;
  }
  if (args.win32_error) {
    msg = std::format(L"{}: ({}) {}", msg, args.win32_error,
                      get_os_err_msg(args.win32_error).get());
  }
  display_error(msg.data());
  return EXIT_FAILURE;
}
