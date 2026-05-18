#include "syscall.h"
#include "../drivers/vga.h"
#include "../process/scheduler.h"

namespace {
    struct __attribute__((packed)) IdtEntry {
        uint16_t isr_low;
        uint16_t kernel_cs;
        uint8_t reserved;
        uint8_t attributes;
        uint16_t isr_high;
    };

    struct __attribute__((packed)) Idtr {
        uint16_t limit;
        uint32_t base;
    };

    uint32_t strlen_kernel(const char* text) {
        uint32_t length = 0;
        if (!text) {
            return 0;
        }
        while (text[length]) {
            ++length;
        }
        return length;
    }

    void set_idt_gate(uint8_t vector, void* handler, uint8_t attributes) {
        Idtr idtr;
        __asm__ volatile("sidt %0" : "=m"(idtr));

        IdtEntry* idt = reinterpret_cast<IdtEntry*>(idtr.base);
        uint32_t address = reinterpret_cast<uint32_t>(handler);

        idt[vector].isr_low = address & 0xFFFF;
        idt[vector].kernel_cs = 0x08;
        idt[vector].reserved = 0;
        idt[vector].attributes = attributes;
        idt[vector].isr_high = (address >> 16) & 0xFFFF;
    }
}

extern "C" uint32_t syscall_dispatch_from_interrupt(SyscallRegisterFrame* frame) {
    if (!frame) {
        return 0;
    }

    switch (frame->eax) {
        case Syscalls::SYS_FORK:
            return Scheduler::fork_current();

        case Syscalls::SYS_GETPID:
            return Scheduler::current_pid();

        case Syscalls::SYS_WRITE:
            VGA::write_len(reinterpret_cast<const char*>(frame->ebx), frame->ecx);
            return frame->ecx;

        default:
            VGA::write("[syscall] unknown number ");
            VGA::write_dec(frame->eax);
            VGA::put_char('\n');
            return 0;
    }
}

namespace Syscalls {
    void install() {
        set_idt_gate(0x80, reinterpret_cast<void*>(&syscall_stub), 0xEE);
    }

    static uint32_t invoke(uint32_t number, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
        uint32_t result;
        __asm__ volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(number), "b"(arg1), "c"(arg2), "d"(arg3)
            : "memory"
        );
        return result;
    }

    uint32_t fork() {
        return invoke(SYS_FORK, 0, 0, 0);
    }

    uint32_t getpid() {
        return invoke(SYS_GETPID, 0, 0, 0);
    }

    uint32_t write(const char* text) {
        return write(text, strlen_kernel(text));
    }

    uint32_t write(const char* text, uint32_t length) {
        return invoke(SYS_WRITE, reinterpret_cast<uint32_t>(text), length, 0);
    }
}
