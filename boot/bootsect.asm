; A boot sector that enters 32- bit protected mode.
[org 0x7c00]
KERNEL_OFFSET equ 0x1000 ; This is the memory offset to which we will load our kernel

    mov [BOOT_DRIVE], dl ; BIOS stores our boot drive in DL , so it ’s
                         ; best to remember this for later.

    mov bp, 0x9000 ; Set the stack.
    mov sp, bp
    
    mov bx, MSG_REAL_MODE
    call print_string

    call load_kernel
    
    call switch_to_pm ; Note that we never return from here.
    
    jmp $

%include "boot/print_string.asm"
%include "boot/disk_load.asm"
%include "boot/gdt.asm"
%include "boot/print_string_pm.asm"
%include "boot/switch_to_pm.asm"

[bits 16]
; load_kernel
load_kernel :
    mov bx , MSG_LOAD_KERNEL ; Print a message to say we are loading the kernel
    call print_string
    
    mov bx , KERNEL_OFFSET ; Set -up parameters for our disk_load routine , so
    mov dh , 31            ; that we load the first 31 sectors ( excluding
    mov dl , [BOOT_DRIVE]  ; the boot sector ) from the boot disk ( i.e. our
                           ; kernel code ) to address KERNEL_OFFSET
    call disk_load
    ret

[bits 32]
; This is where we arrive after switching to and initialising protected mode.
BEGIN_PM :
    mov ebp , 0x90000 ; Update our stack position so it is right
    mov esp , ebp ; at the top of the free space (which is 572KB, more than enough for a kernel)

    mov ebx , MSG_PROT_MODE
    call print_string_pm ; Use our 32- bit print routine.

    call KERNEL_OFFSET ; Now jump to the address of our loaded
                       ; kernel code , assume the brace position ,
                       ; and cross your fingers. Here we go!

    jmp $ ; Hang.

; Global variables
BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Successfully landed in 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory.", 0

; Bootsector padding
times 510 -($-$$) db 0
dw 0xaa55