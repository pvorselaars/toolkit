.intel_syntax noprefix

.global _start

_start:
    xor rbp, rbp
    mov rdi, [rsp]
    lea rsi, [rsp+8]
    lea rdx, [rsp+rdi*8+16]
    call main
    mov rdi, rax
    call exit
