[bits 32]
section .text

global arch_gdt_flush
global arch_tss_flush
global arch_enter_user

arch_gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush
.flush:
    ret

arch_tss_flush:
    mov ax, 0x28
    ltr ax
    ret

arch_enter_user:
    mov eax, [esp + 4]     ; user entry point
    mov edx, [esp + 8]     ; user stack top

    mov bx, 0x23
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    push dword 0x23
    push edx
    pushfd
    pop ecx
    or ecx, 0x200          ; keep maskable interrupts enabled in user mode
    push ecx
    push dword 0x1B
    push eax
    iretd
