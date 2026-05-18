#include "timer.h"
#include "../core/interrupts.h"
#include "../core/pic.h"
#include "../core/portio.h"
#include "../process/scheduler.h"

using namespace PortIO;

namespace Timer {
    constexpr uint32_t PIT_BASE_FREQUENCY = 1193180;
    constexpr uint16_t PIT_COMMAND_PORT = 0x43;
    constexpr uint16_t PIT_CHANNEL0_PORT = 0x40;
    constexpr uint8_t IRQ_NUMBER = 0;
    constexpr uint8_t IDT_VECTOR = 32;

    static volatile uint32_t tick_count = 0;
    static volatile bool quantum_ready = false;
    static interruption timer_interrupt;

    static void handler() {
        ++tick_count;
        if (Scheduler::on_timer_tick()) {
            quantum_ready = true;
        }
        PIC::acknowledge(IRQ_NUMBER);
    }

    void init(uint32_t frequency_hz) {
        if (frequency_hz == 0) {
            frequency_hz = 50;
        }

        uint32_t divisor = PIT_BASE_FREQUENCY / frequency_hz;

        timer_interrupt.code = IDT_VECTOR;
        timer_interrupt.callback = reinterpret_cast<uint32_t>(&handler);
        InterruptManager::add_interrupt(&timer_interrupt);

        outb(PIT_COMMAND_PORT, 0x36);
        outb(PIT_CHANNEL0_PORT, static_cast<uint8_t>(divisor & 0xFF));
        outb(PIT_CHANNEL0_PORT, static_cast<uint8_t>((divisor >> 8) & 0xFF));

        PIC::enable_irq(IRQ_NUMBER);
    }

    uint32_t ticks() {
        return tick_count;
    }

    bool quantum_elapsed() {
        return quantum_ready;
    }

    void clear_quantum_flag() {
        quantum_ready = false;
    }

    void sleep_ticks(uint32_t count) {
        uint32_t target = tick_count + count;
        while (tick_count < target) {
            __asm__ volatile("hlt");
        }
    }
}
