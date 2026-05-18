#include "core/ktypes.h"
#include "core/interrupts.h"
#include "core/pic.h"
#include "core/portio.h"

#include "arch/gdt.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "drivers/vga.h"
#include "mm/page_allocator.h"
#include "process/scheduler.h"
#include "selftest.h"
#include "syscalls/syscall.h"
#include "user_program.h"

using namespace PortIO;

extern "C" int kmain() {
    disable_interrupts();

    VGA::init();
    VGA::write_line("MiniOS skeleton starting...");

    GDT::init();
    VGA::write_line("GDT/TSS ready: kernel Ring 0 and user Ring 3 descriptors installed.");

    InterruptManager::init();
    PIC::remap(32, 40);
    Syscalls::install();
    VGA::write_line("IDT/PIC ready: syscall gate int 0x80 installed.");

    PageAllocator::init();
    Scheduler::init(5);

    Timer::init(50);
    Keyboard::init();

    enable_interrupts();

    SelfTest::run_all();

    Scheduler::PCB* user_process = Scheduler::create_process("user-main", 0);
    if (!user_process) {
        VGA::write_line("[panic] could not allocate PCB/pages for user process");
        while (true) {
            __asm__ volatile("hlt");
        }
    }

    Scheduler::set_current_pid(user_process->pid);

    void* user_stack = reinterpret_cast<void*>(user_process->pages[0] + PageAllocator::PAGE_SIZE);

    VGA::write_line("Transitioning to user mode with iret...");
    GDT::enter_user_mode(user_main, user_stack);

    while (true) {
        __asm__ volatile("hlt");
    }

    return 0;
}
