#include "gdt.h"

namespace {
    struct __attribute__((packed)) GdtEntry {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    };

    struct __attribute__((packed)) GdtPointer {
        uint16_t limit;
        uint32_t base;
    };

    struct __attribute__((packed)) TssEntry {
        uint32_t prev_tss;
        uint32_t esp0;
        uint32_t ss0;
        uint32_t esp1;
        uint32_t ss1;
        uint32_t esp2;
        uint32_t ss2;
        uint32_t cr3;
        uint32_t eip;
        uint32_t eflags;
        uint32_t eax;
        uint32_t ecx;
        uint32_t edx;
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint32_t es;
        uint32_t cs;
        uint32_t ss;
        uint32_t ds;
        uint32_t fs;
        uint32_t gs;
        uint32_t ldt;
        uint16_t trap;
        uint16_t iomap_base;
    };

    static GdtEntry gdt[6];
    static GdtPointer gdtr;
    static TssEntry tss;
    static uint8_t kernel_stack[4096];

    void memory_set(void* destination, uint8_t value, uint32_t length) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(destination);
        for (uint32_t i = 0; i < length; ++i) {
            bytes[i] = value;
        }
    }

    void set_gate(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
        gdt[index].base_low = base & 0xFFFF;
        gdt[index].base_middle = (base >> 16) & 0xFF;
        gdt[index].base_high = (base >> 24) & 0xFF;

        gdt[index].limit_low = limit & 0xFFFF;
        gdt[index].granularity = (limit >> 16) & 0x0F;
        gdt[index].granularity |= granularity & 0xF0;
        gdt[index].access = access;
    }

    void write_tss(uint32_t index, uint16_t selector, uint32_t esp0) {
        uint32_t base = reinterpret_cast<uint32_t>(&tss);
        uint32_t limit = sizeof(TssEntry) - 1;

        set_gate(index, base, limit, 0x89, 0x40);
        memory_set(&tss, 0, sizeof(TssEntry));

        tss.ss0 = selector;
        tss.esp0 = esp0;
        tss.cs = GDT::USER_CODE;
        tss.ss = GDT::USER_DATA;
        tss.ds = GDT::USER_DATA;
        tss.es = GDT::USER_DATA;
        tss.fs = GDT::USER_DATA;
        tss.gs = GDT::USER_DATA;
        tss.iomap_base = sizeof(TssEntry);
    }
}

namespace GDT {
    void init() {
        gdtr.limit = sizeof(gdt) - 1;
        gdtr.base = reinterpret_cast<uint32_t>(&gdt);

        set_gate(0, 0, 0, 0, 0);
        set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);
        set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);
        set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF);
        set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF);
        write_tss(5, KERNEL_DATA, reinterpret_cast<uint32_t>(kernel_stack + sizeof(kernel_stack)));

        arch_gdt_flush(&gdtr);
        arch_tss_flush();
    }

    void enter_user_mode(void (*entry)(), void* user_stack_top) {
        arch_enter_user(entry, user_stack_top);
    }
}
