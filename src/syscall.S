.intel_syntax noprefix

.global read
.global write
.global open
.global close
.global fork
.global execve
.global exit
.global mount
.global waitid
.global fstatat

read:
mov rax, 0
syscall
ret

write:
mov rax, 1
syscall
ret

open:
mov rax, 2
syscall
ret

close:
mov rax, 3
syscall
ret

fork:
mov rax, 57
syscall
ret

execve:
mov rax, 59
syscall
ret

exit:
mov rax, 60
syscall
ret

mount:
mov rax, 165
syscall
ret

waitid:
mov rax, 247
mov r10, rcx
syscall
ret

fstatat:
mov rax, 262
mov r10, rcx
syscall
ret