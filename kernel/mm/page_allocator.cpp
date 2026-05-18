#include "page_allocator.h"

namespace PageAllocator {
    constexpr uint32_t POOL_BASE = 0x50000;
    constexpr uint32_t POOL_PAGE_COUNT = 48;

    static bool used[POOL_PAGE_COUNT];

    static void zero_page(uint8_t* page) {
        for (uint32_t i = 0; i < PAGE_SIZE; ++i) {
            page[i] = 0;
        }
    }

    void init() {
        for (uint32_t i = 0; i < POOL_PAGE_COUNT; ++i) {
            used[i] = false;
        }
    }

    void* alloc_page() {
        for (uint32_t i = 0; i < POOL_PAGE_COUNT; ++i) {
            if (!used[i]) {
                used[i] = true;
                uint8_t* page = reinterpret_cast<uint8_t*>(POOL_BASE + (i * PAGE_SIZE));
                zero_page(page);
                return page;
            }
        }
        return 0;
    }

    void free_page(void* page) {
        if (!page) {
            return;
        }

        uint32_t address = reinterpret_cast<uint32_t>(page);
        if (address < POOL_BASE) {
            return;
        }

        uint32_t offset = address - POOL_BASE;
        if ((offset % PAGE_SIZE) != 0) {
            return;
        }

        uint32_t index = offset / PAGE_SIZE;
        if (index < POOL_PAGE_COUNT) {
            used[index] = false;
        }
    }

    bool alloc_process_pages(uint32_t pages[PAGES_PER_PROCESS]) {
        for (uint32_t i = 0; i < PAGES_PER_PROCESS; ++i) {
            void* page = alloc_page();
            if (!page) {
                for (uint32_t rollback = 0; rollback < i; ++rollback) {
                    free_page(reinterpret_cast<void*>(pages[rollback]));
                    pages[rollback] = 0;
                }
                return false;
            }
            pages[i] = reinterpret_cast<uint32_t>(page);
        }
        return true;
    }

    uint32_t free_pages() {
        uint32_t count = 0;
        for (uint32_t i = 0; i < POOL_PAGE_COUNT; ++i) {
            if (!used[i]) {
                ++count;
            }
        }
        return count;
    }

    uint32_t total_pages() {
        return POOL_PAGE_COUNT;
    }
}
