#include "scheduler.h"
#include "../drivers/vga.h"

namespace Scheduler {
    constexpr uint32_t MAX_PROCESSES = 8;

    static PCB processes[MAX_PROCESSES];
    static uint32_t next_pid = 1;
    static int current_index = -1;
    static uint32_t ticks_in_quantum = 0;
    static uint32_t quantum_limit = 5;

    static int find_process_index(uint32_t pid) {
        for (uint32_t i = 0; i < MAX_PROCESSES; ++i) {
            if (processes[i].state != UNUSED && processes[i].pid == pid) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    static int find_free_slot() {
        for (uint32_t i = 0; i < MAX_PROCESSES; ++i) {
            if (processes[i].state == UNUSED || processes[i].state == TERMINATED) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    static int find_next_ready() {
        for (uint32_t step = 1; step <= MAX_PROCESSES; ++step) {
            uint32_t index = static_cast<uint32_t>((current_index + step + MAX_PROCESSES) % MAX_PROCESSES);
            if (processes[index].state == READY) {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    static void free_process_pages(PCB* process) {
        for (uint32_t i = 0; i < PageAllocator::PAGES_PER_PROCESS; ++i) {
            if (process->pages[i]) {
                PageAllocator::free_page(reinterpret_cast<void*>(process->pages[i]));
                process->pages[i] = 0;
            }
        }
    }

    void init(uint32_t quantum_ticks) {
        for (uint32_t i = 0; i < MAX_PROCESSES; ++i) {
            processes[i].pid = 0;
            processes[i].state = UNUSED;
            processes[i].name = "";
            processes[i].entry = 0;
            processes[i].run_count = 0;
            for (uint32_t p = 0; p < PageAllocator::PAGES_PER_PROCESS; ++p) {
                processes[i].pages[p] = 0;
            }
        }
        next_pid = 1;
        current_index = -1;
        ticks_in_quantum = 0;
        quantum_limit = quantum_ticks ? quantum_ticks : 5;
    }

    PCB* create_process(const char* name, void (*entry)(PCB*)) {
        int slot = find_free_slot();
        if (slot < 0) {
            return 0;
        }

        PCB* process = &processes[slot];
        process->pid = next_pid++;
        process->state = READY;
        process->name = name ? name : "process";
        process->entry = entry;
        process->run_count = 0;

        if (!PageAllocator::alloc_process_pages(process->pages)) {
            process->state = UNUSED;
            process->pid = 0;
            return 0;
        }

        return process;
    }

    PCB* current() {
        if (current_index < 0) {
            return 0;
        }
        return &processes[current_index];
    }

    uint32_t current_pid() {
        PCB* process = current();
        return process ? process->pid : 0;
    }

    uint32_t fork_current() {
        PCB* parent = current();
        const char* child_name = parent ? parent->name : "fork-child";
        void (*child_entry)(PCB*) = parent ? parent->entry : 0;
        PCB* child = create_process(child_name, child_entry);
        return child ? child->pid : 0;
    }

    bool on_timer_tick() {
        ++ticks_in_quantum;
        if (ticks_in_quantum >= quantum_limit) {
            ticks_in_quantum = 0;
            return true;
        }
        return false;
    }

    void dispatch_next() {
        int next = find_next_ready();
        if (next < 0) {
            return;
        }

        current_index = next;
        PCB* process = &processes[next];
        process->state = RUNNING;
        ++process->run_count;

        VGA::write("[dispatch] pid=");
        VGA::write_dec(process->pid);
        VGA::write(" name=");
        VGA::write(process->name);
        VGA::write(" run=");
        VGA::write_dec(process->run_count);
        VGA::write(" pages=");
        VGA::write_hex(process->pages[0]);
        VGA::put_char(' ');
        VGA::write_hex(process->pages[1]);
        VGA::put_char(' ');
        VGA::write_hex(process->pages[2]);
        VGA::put_char('\n');

        if (process->entry) {
            process->entry(process);
        }

        if (process->state == RUNNING) {
            process->state = READY;
        }

        if (process->state == TERMINATED) {
            free_process_pages(process);
        }
    }

    void run_demo_rounds(uint32_t rounds) {
        for (uint32_t i = 0; i < rounds; ++i) {
            dispatch_next();
        }
    }

    void set_current_pid(uint32_t pid) {
        int index = find_process_index(pid);
        if (index >= 0) {
            current_index = index;
        }
    }
}
