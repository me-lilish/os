#pragma once
#include "../core/ktypes.h"
#include "../mm/page_allocator.h"

namespace Scheduler {
    enum ProcessState : uint8_t {
        UNUSED = 0,
        READY,
        RUNNING,
        BLOCKED,
        TERMINATED
    };

    struct PCB {
        uint32_t pid;
        ProcessState state;
        const char* name;
        void (*entry)(PCB*);
        uint32_t pages[PageAllocator::PAGES_PER_PROCESS];
        uint32_t run_count;
    };

    void init(uint32_t quantum_ticks = 5);
    PCB* create_process(const char* name, void (*entry)(PCB*));
    PCB* current();
    uint32_t current_pid();
    uint32_t fork_current();
    bool on_timer_tick();
    void dispatch_next();
    void run_demo_rounds(uint32_t rounds);
    void set_current_pid(uint32_t pid);
}
