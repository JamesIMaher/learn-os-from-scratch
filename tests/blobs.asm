; =============================================================================
; blobs.asm - tiny ring-3 programs for the lesson 12 checkpoints.
;
; These are not executed where they sit: lesson12.c copies each one into a
; fresh user page at 0x40000000 and runs it there in ring 3. That's why they
; find their own data with the call/pop trick instead of absolute addresses.
; They use the system call ABI from include/abi/syscall_nums.h directly.
; =============================================================================
bits 32
section .rodata

%define SYS_EXIT   0
%define SYS_WRITE  1
%define SYS_YIELD  3
%define SYS_GETPID 4
%define SYS_SLEEP  5
%define DATA_PAGE  0x40001000

%macro BLOB 1
global blob_%1, blob_%1_end
%endmacro

; write a message to fd 1, exit with write's return value
BLOB hello
blob_hello:
    call .here
.here:
    pop esi
    mov eax, SYS_WRITE
    mov ebx, 1
    lea ecx, [esi + .msg - .here]
    mov edx, .msg_end - .msg
    int 0x80
    mov ebx, eax
    mov eax, SYS_EXIT
    int 0x80
    jmp $
.msg: db "ring3-hello-9c1", 10
.msg_end:
blob_hello_end:

; system calls must reject bad arguments; exits with 0x55 if they all do,
; or the number of the first check that went wrong
BLOB badargs
blob_badargs:
    call .here
.here:
    pop esi
    mov eax, SYS_WRITE          ; 1: a kernel pointer -> -EFAULT
    mov ebx, 1
    mov ecx, 0x00100000
    mov edx, 8
    int 0x80
    mov ebx, 1
    cmp eax, -14
    jne .fail
    mov eax, SYS_WRITE          ; 2: runs off the end of mapped memory -> -EFAULT
    mov ebx, 1
    mov ecx, DATA_PAGE + 0xFF8
    mov edx, 16
    int 0x80
    mov ebx, 2
    cmp eax, -14
    jne .fail
    mov eax, SYS_WRITE          ; 3: wraps around the address space -> -EFAULT
    mov ebx, 1
    mov ecx, DATA_PAGE
    mov edx, 0xFFFFFFF0
    int 0x80
    mov ebx, 3
    cmp eax, -14
    jne .fail
    mov eax, SYS_WRITE          ; 4: bad file descriptor -> -EBADF
    mov ebx, 7
    lea ecx, [esi]
    mov edx, 1
    int 0x80
    mov ebx, 4
    cmp eax, -9
    jne .fail
    mov eax, 999                ; 5: no such system call -> -ENOSYS
    int 0x80
    mov ebx, 5
    cmp eax, -38
    jne .fail
    mov eax, SYS_WRITE          ; 6: zero bytes is fine -> 0
    mov ebx, 1
    lea ecx, [esi]
    xor edx, edx
    int 0x80
    mov ebx, 6
    cmp eax, 0
    jne .fail
    mov ebx, 0x55
.fail:
    mov eax, SYS_EXIT
    int 0x80
    jmp $
blob_badargs_end:

; a privileged instruction in ring 3 -> #GP -> the thread is killed
BLOB cli
blob_cli:
    cli
    mov eax, SYS_EXIT
    xor ebx, ebx
    int 0x80
    jmp $
blob_cli_end:

; reading kernel memory from ring 3 -> #PF -> the thread is killed
BLOB peek
blob_peek:
    mov eax, [0x00100000]
    mov ebx, eax
    mov eax, SYS_EXIT
    int 0x80
    jmp $
blob_peek_end:

; jumping into kernel code from ring 3 -> #PF -> killed
BLOB jump
blob_jump:
    mov eax, 0x00100000
    jmp eax
blob_jump_end:

; getpid, yield, sleep; exits with getpid's result
BLOB pid
blob_pid:
    mov eax, SYS_YIELD
    int 0x80
    mov edi, eax                ; yield returns 0
    mov eax, SYS_SLEEP
    mov ebx, 50
    int 0x80
    or edi, eax                 ; sleep returns 0
    mov eax, SYS_GETPID
    int 0x80
    mov ebx, eax
    test edi, edi
    jz .ok
    mov ebx, -1
.ok:
    mov eax, SYS_EXIT
    int 0x80
    jmp $
blob_pid_end:

; count in DATA_PAGE[0] without ever making a system call, until the kernel
; sets DATA_PAGE[4]. Only preemption can get the kernel to run meanwhile.
BLOB spin
blob_spin:
.loop:
    inc dword [DATA_PAGE]
    cmp dword [DATA_PAGE + 4], 0
    je .loop
    mov eax, SYS_EXIT
    mov ebx, 0x77
    int 0x80
    jmp $
blob_spin_end:

; check that registers survive a system call (only EAX may change)
BLOB regs
blob_regs:
    mov ebx, 1
    mov ecx, 0
    mov edx, 0
    mov esi, 0x51515151
    mov edi, 0xD1D1D1D1
    mov ebp, 0xB0B0B0B0
    mov eax, SYS_WRITE          ; write(1, NULL, 0): harmless
    int 0x80
    mov eax, SYS_YIELD
    int 0x80
    mov ebx, 1
    cmp esi, 0x51515151
    jne .out
    cmp edi, 0xD1D1D1D1
    jne .out
    cmp ebp, 0xB0B0B0B0
    jne .out
    mov ebx, 0x42
.out:
    mov eax, SYS_EXIT
    int 0x80
    jmp $
blob_regs_end:
