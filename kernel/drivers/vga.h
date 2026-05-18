#pragma once
#include "../core/ktypes.h"

namespace VGA {
    enum Color : uint8_t {
        BLACK = 0,
        BLUE = 1,
        GREEN = 2,
        CYAN = 3,
        RED = 4,
        MAGENTA = 5,
        BROWN = 6,
        LIGHT_GREY = 7,
        DARK_GREY = 8,
        LIGHT_BLUE = 9,
        LIGHT_GREEN = 10,
        LIGHT_CYAN = 11,
        LIGHT_RED = 12,
        LIGHT_MAGENTA = 13,
        LIGHT_BROWN = 14,
        WHITE = 15
    };

    void init();
    void clear();
    void set_color(uint8_t foreground, uint8_t background = BLACK);
    void put_char(char c);
    void write(const char* text);
    void write_len(const char* text, uint32_t length);
    void write_line(const char* text);
    void write_dec(uint32_t value);
    void write_hex(uint32_t value);
}
