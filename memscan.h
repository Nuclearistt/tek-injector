//===-- memscan.h - Memory scan function declaration ----------------------===//
//
// Copyright (c) 2025 Nuclearist
// Part of TEK Injector, under the MIT License
// See https://github.com/Nuclearistt/tek-injector/blob/main/LICENSE for
//    license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//
#pragma once

#include <stddef.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t memscan(const void *mem, size_t mem_size, const void *str,
               size_t str_size);

#ifdef __cplusplus
} // extern "C"
#endif

