[bits 32]
section .text

extern syscall_dispatch_from_interrupt
global syscall_stub

syscall_stub:
    pushad

    mov eax, esp
    push eax
    call syscall_dispatch_from_interrupt
    add esp, 4

    ; pushad saves EAX at [esp + 28]. Replacing it makes EAX the syscall return
    ; value after popad restores the user register frame.
    mov [esp + 28], eax

    popad
    iretd
