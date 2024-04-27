//===-- vm-stubs.s - Stub wrappers for virtual methods --------------------===//
//
// Copyright (c) 2024 Nuclearist
// Part of TEK Injector, under the MIT License
// See https://github.com/Nuclearistt/tek-injector/blob/main/LICENSE for
//    license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//
//
// This file provides vmn_stub methods for C++ interface wrappers, that pass
//    execution to n'th method of wrapped interface with the same arguments.
//
//===----------------------------------------------------------------------===//

.text

.global vm0_stub // Make methods visible to linker
.balign 16, 0xCC // Align methods on 16-byte boundary with INT3 padding
vm0_stub:
// Load wrapped interface pointer (cpp_interface_wrapper->original_interface)
//    as first argument
mov 8(%rcx), %rcx
// Load wrapped interface's virtual method table pointer into RAX
mov (%rcx), %rax
// Pass execution to n'th method, other arguments remain the same since no other
//    registers were modified
jmpq *0(%rax)

.global vm1_stub
.balign 16, 0xCC
vm1_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x8(%rax)

.global vm2_stub
.balign 16, 0xCC
vm2_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x10(%rax)

.global vm3_stub
.balign 16, 0xCC
vm3_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x18(%rax)

.global vm4_stub
.balign 16, 0xCC
vm4_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x20(%rax)

.global vm5_stub
.balign 16, 0xCC
vm5_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x28(%rax)

.global vm6_stub
.balign 16, 0xCC
vm6_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x30(%rax)

.global vm7_stub
.balign 16, 0xCC
vm7_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x38(%rax)

.global vm8_stub
.balign 16, 0xCC
vm8_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x40(%rax)

.global vm9_stub
.balign 16, 0xCC
vm9_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x48(%rax)

.global vm10_stub
.balign 16, 0xCC
vm10_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x50(%rax)

.global vm11_stub
.balign 16, 0xCC
vm11_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x58(%rax)

.global vm12_stub
.balign 16, 0xCC
vm12_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x60(%rax)

.global vm13_stub
.balign 16, 0xCC
vm13_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x68(%rax)

.global vm14_stub
.balign 16, 0xCC
vm14_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x70(%rax)

.global vm15_stub
.balign 16, 0xCC
vm15_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x78(%rax)

.global vm16_stub
.balign 16, 0xCC
vm16_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x80(%rax)

.global vm17_stub
.balign 16, 0xCC
vm17_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x88(%rax)

.global vm18_stub
.balign 16, 0xCC
vm18_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x90(%rax)

.global vm19_stub
.balign 16, 0xCC
vm19_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x98(%rax)

.global vm20_stub
.balign 16, 0xCC
vm20_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xA0(%rax)

.global vm21_stub
.balign 16, 0xCC
vm21_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xA8(%rax)

.global vm22_stub
.balign 16, 0xCC
vm22_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xB0(%rax)

.global vm23_stub
.balign 16, 0xCC
vm23_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xB8(%rax)

.global vm24_stub
.balign 16, 0xCC
vm24_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xC0(%rax)

.global vm25_stub
.balign 16, 0xCC
vm25_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xC8(%rax)

.global vm26_stub
.balign 16, 0xCC
vm26_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xD0(%rax)

.global vm27_stub
.balign 16, 0xCC
vm27_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xD8(%rax)

.global vm28_stub
.balign 16, 0xCC
vm28_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xE0(%rax)

.global vm29_stub
.balign 16, 0xCC
vm29_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xE8(%rax)

.global vm30_stub
.balign 16, 0xCC
vm30_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xF0(%rax)

.global vm31_stub
.balign 16, 0xCC
vm31_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0xF8(%rax)

.global vm32_stub
.balign 16, 0xCC
vm32_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x100(%rax)

.global vm33_stub
.balign 16, 0xCC
vm33_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x108(%rax)

.global vm34_stub
.balign 16, 0xCC
vm34_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x110(%rax)

.global vm35_stub
.balign 16, 0xCC
vm35_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x118(%rax)

.global vm36_stub
.balign 16, 0xCC
vm36_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x120(%rax)

.global vm37_stub
.balign 16, 0xCC
vm37_stub:
mov 8(%rcx), %rcx
mov (%rcx), %rax
jmpq *0x128(%rax)
