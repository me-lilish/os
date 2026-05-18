#pragma once
#include "../core/ktypes.h"

namespace PageAllocator {
    constexpr uint32_t PAGE_SIZE = 4096;
    constexpr uint32_t PAGES_PER_PROCESS = 3;

    void init();
    void* alloc_page();
    void free_page(void* page);
    bool alloc_process_pages(uint32_t pages[PAGES_PER_PROCESS]);
    uint32_t free_pages();
    uint32_t total_pages();
}
