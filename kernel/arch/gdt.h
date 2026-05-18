#pragma once
#include "../core/ktypes.h"

namespace GDT {
    constexpr uint16_t KERNEL_CODE = 0x08;
    constexpr uint16_t KERNEL_DATA = 0x10;
    constexpr uint16_t USER_CODE = 0x1B;
    constexpr uint16_t USER_DATA = 0x23;
    constexpr uint16_t TSS_SELECTOR = 0x28;

    void init();
    void enter_user_mode(void (*entry)(), void* user_stack_top);
}

extern "C" void arch_gdt_flush(void* gdtr);
extern "C" void arch_tss_flush();
extern "C" void arch_enter_user(void (*entry)(), void* user_stack_top);
