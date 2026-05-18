#pragma once
#include "../core/ktypes.h"

namespace Syscalls {
    enum Number : uint32_t {
        SYS_FORK = 1,
        SYS_GETPID = 2,
        SYS_WRITE = 3
    };

    void install();
    uint32_t fork();
    uint32_t getpid();
    uint32_t write(const char* text);
    uint32_t write(const char* text, uint32_t length);
}

struct SyscallRegisterFrame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t original_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};

extern "C" uint32_t syscall_dispatch_from_interrupt(SyscallRegisterFrame* frame);
extern "C" void syscall_stub();
