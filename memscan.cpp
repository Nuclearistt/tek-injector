//===-- memscan.cpp - Memory scan function implementation -----------------===//
//
// Copyright (c) 2025 Nuclearist
// Part of TEK Injector, under the MIT License
// See https://github.com/Nuclearistt/tek-injector/blob/main/LICENSE for
//    license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//
#include "memscan.h"

#include <string_view>

size_t memscan(const void *mem, size_t mem_size, const void *str,
               size_t str_size) {
  return std::string_view(reinterpret_cast<const char *>(mem), mem_size)
      .find(reinterpret_cast<const char *>(str), 0, str_size);
}

