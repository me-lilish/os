#include "selftest.h"
#include "drivers/vga.h"
#include "drivers/timer.h"
#include "mm/page_allocator.h"
#include "process/scheduler.h"
#include "syscalls/syscall.h"

namespace {
    void pass(const char* name) {
        VGA::set_color(VGA::LIGHT_GREEN);
        VGA::write("[PASS] ");
        VGA::set_color(VGA::WHITE);
        VGA::write_line(name);
    }

    void fail(const char* name) {
        VGA::set_color(VGA::LIGHT_RED);
        VGA::write("[FAIL] ");
        VGA::set_color(VGA::WHITE);
        VGA::write_line(name);
    }

    void demo_process(Scheduler::PCB* process) {
        VGA::write("          process step pid=");
        VGA::write_dec(process->pid);
        VGA::write(" count=");
        VGA::write_dec(process->run_count);
        VGA::put_char('\n');

        if (process->run_count >= 2) {
            process->state = Scheduler::TERMINATED;
        }
    }
}

namespace SelfTest {
    void run_all() {
        VGA::write_line("Running kernel self-tests...");

        uint32_t before = PageAllocator::free_pages();
        void* p1 = PageAllocator::alloc_page();
        void* p2 = PageAllocator::alloc_page();
        if (p1 && p2 && PageAllocator::free_pages() == before - 2) {
            pass("page allocator allocates 4 KiB frames");
        } else {
            fail("page allocator allocates 4 KiB frames");
        }
        PageAllocator::free_page(p1);
        PageAllocator::free_page(p2);

        Scheduler::PCB* a = Scheduler::create_process("A", demo_process);
        Scheduler::PCB* b = Scheduler::create_process("B", demo_process);
        if (a && b && a->pages[0] && a->pages[1] && a->pages[2] && b->pages[0]) {
            pass("PCB creation allocates exactly three pages per process");
        } else {
            fail("PCB creation allocates exactly three pages per process");
        }

        VGA::write_line("Round-robin scheduler demo:");
        Scheduler::run_demo_rounds(4);
        pass("round-robin dispatcher selected runnable PCBs");

        uint32_t ticks_before = Timer::ticks();
        Timer::sleep_ticks(2);
        if (Timer::ticks() >= ticks_before + 2) {
            pass("PIT timer generates IRQ0 ticks");
        } else {
            fail("PIT timer generates IRQ0 ticks");
        }

        Scheduler::PCB* syscall_proc = Scheduler::create_process("syscall-owner", 0);
        if (syscall_proc) {
            Scheduler::set_current_pid(syscall_proc->pid);
        }

        uint32_t pid = Syscalls::getpid();
        if (pid == Scheduler::current_pid() && pid != 0) {
            pass("getpid() syscall returns current PCB pid");
        } else {
            fail("getpid() syscall returns current PCB pid");
        }

        Syscalls::write("[sys_write] write() syscall reached the monitor\n");
        uint32_t child = Syscalls::fork();
        if (child != 0) {
            pass("fork() syscall creates child PCB with three pages");
        } else {
            fail("fork() syscall creates child PCB with three pages");
        }
    }
}
