[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_string_pm :
    pusha
    mov edx , VIDEO_MEMORY ; edx is used to access the video memory

print_string_pm_loop :
    mov al , [ ebx ] ; ebx is the one used to get the string from memory
    mov ah , WHITE_ON_BLACK
    cmp al , 0
    je print_string_pm_done
    mov [edx], ax

    add ebx , 1
    add edx , 2 ; each character takes 2 bytes in the video memory
    jmp print_string_pm_loop

print_string_pm_done :
    popa
    ret