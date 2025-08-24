//===-- tek-injector.h - TEK Injector API declarations --------------------===//
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
///  Declarations of TEK Injector API macros, types and functions.
///
//===----------------------------------------------------------------------===//
#pragma once

#include <stdint.h>

//===-- Compiler macros ---------------------------------------------------===//

#ifndef __clang__
// Clang nullability attributes are replaced with mock macros for other
//    compilers.

#ifndef _Nullable
#define _Nullable
#endif // ndef _Nullable
#ifndef _Nonnull
#define _Nonnull
#endif // ndef _Nonnull
#ifndef _Null_unspecified
#define _Null_unspecified
#endif // ndef _Null_unspecified

#endif // ndef __clang__

#ifdef TEK_INJ_IMPL
// Reduce windows.h API set reduced as much as possible
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0xA000
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#endif // def TEK_INJ_IMPL
#include <windows.h>

// Public API attribute.
#ifdef TEK_INJ_STATIC
#define TEK_INJ_API visibility("default")
#else // def TEK_INJ_STATIC

// Use DLL exports/imports.
#ifdef TEK_INJ_EXPORT
#define TEK_INJ_API dllexport
#else // def TEK_INJ_EXPORT
#define TEK_INJ_API dllimport
#endif // def TEK_INJ_EXPORT else

#endif // def TEK_INJ_STATIC else

//===-- Types -------------------------------------------------------------===//

/// Supported methods for TEK Game Runtime to load settings.
enum tek_gr_load_type {
  /// Settings file path is received via file mapping, tek-game-runtime reads
  ///    that file.
  TEK_GR_LOAD_TYPE_file_path,
  /// Settings JSON content is received from the file mapping directly.
  TEK_GR_LOAD_TYPE_data
};
/// @copydoc tek_gr_load_type
typedef enum tek_gr_load_type tek_gr_load_type;

/// Injection flags.
enum [[clang::flag_enum]] tek_inj_flag {
  TEK_INJ_FLAG_none,
  /// Set game process priority to high.
  TEK_INJ_FLAG_high_proc_prio = 1 << 0,
  /// Run game process elevated if the calling process is elevated as well. By
  ///    default, game process is always started without admin privileges.
  TEK_INJ_FLAG_run_as_admin = 1 << 1
};
/// @copydoc tek_inj_flag
typedef enum tek_inj_flag tek_inj_flag;

/// Injection result codes.
enum tek_inj_res {
  /// (0) Injection performed successfully.
  TEK_INJ_RES_ok,
  /// (1) Failed to get process token information.
  TEK_INJ_RES_get_token_info,
  /// (2) Failed to open current process token.
  TEK_INJ_RES_open_token,
  /// (3) Failed to duplicate process token.
  TEK_INJ_RES_duplicate_token,
  /// (4) Failed to set token information.
  TEK_INJ_RES_set_token_info,
  /// (5) Failed to create game process.
  TEK_INJ_RES_create_process,
  /// (6) Failed to allocate memory in game process.
  TEK_INJ_RES_mem_alloc,
  /// (7) Failed to write to game process memory.
  TEK_INJ_RES_mem_write,
  /// (8) Failed to setup security descriptor for the file mapping.
  TEK_INJ_RES_sec_desc,
  /// (9) Failed to create file mapping.
  TEK_INJ_RES_create_mapping,
  /// (10) Failed to map view of the file mapping.
  TEK_INJ_RES_map_view,
  /// (11) Failed to create injection thread.
  TEK_INJ_RES_create_thread,
  /// (12) Failed to wait for injection thread to finish.
  TEK_INJ_RES_thread_wait,
  /// (13) DLL failed to load.
  TEK_INJ_RES_dll_load,
  /// (14) Failed to resume game's main thread.
  TEK_INJ_RES_resume_thread
};
/// @copydoc tek_inj_res
typedef enum tek_inj_res tek_inj_res;

/// Input/output arguments for @ref tek_inj_run_game.
typedef struct tek_inj_game_args tek_inj_game_args;
/// @copydoc tek_inj_game_args
struct tek_inj_game_args {
  /// [In] Path to the game executable to run.
  LPCWSTR _Nonnull exe_path;
  /// [In, optional] Path to the current directory to set for game process. If
  ///    `nullptr`, current directory of the calling process is used.
  LPCWSTR _Nullable current_dir;
  /// [In] Path to libtek-game-runtime.dll to inject. If it's a relative path,
  ///    it must be relative to @ref current_dir.
  LPCWSTR _Nonnull dll_path;
  /// [In] Settings loading type for TEK Game Runtime.
  tek_gr_load_type type;
  /// [In] Number of elements in @ref argv.
  int argc;
  /// [In] Array of command-line arguments to pass to the game process.
  /// Executable
  ///    path is prepended to it automatically.
  const LPCWSTR _Nonnull *_Nullable argv;
  /// [In] Injection flags.
  tek_inj_flag flags;
  /// [In] Size of the buffer passed as @ref data, in bytes.
  uint32_t data_size;
  /// [In] Pointer to the data to pass to TEK Game Runtime. Type of data depends
  ///    on @ref type.
  const char *_Nullable data;
  /// [Out] Injection result code.
  tek_inj_res result;
  /// [Out] If an error occurs, Win32 error code for it.
  DWORD win32_error;
};

//===-- Function ----------------------------------------------------------===//

#ifdef __cplusplus
extern "C" {
#endif // def __cplusplus

/// Start game process and inject TEK Game Runtime into it.
///
/// @param [in, out] args
///    Input/output arguments for the function.
[[gnu::TEK_INJ_API]]
void tek_inj_run_game(tek_inj_game_args *_Nonnull args);

#ifdef __cplusplus
} // extern "C"
#endif // def __cplusplus
