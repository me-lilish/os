#include "vga.h"

namespace VGA {
    constexpr uint32_t WIDTH = 80;
    constexpr uint32_t HEIGHT = 25;
    constexpr uint32_t VIDEO_ADDRESS = 0xB8000;

    static volatile uint16_t* const video = reinterpret_cast<volatile uint16_t*>(VIDEO_ADDRESS);
    static uint32_t row = 0;
    static uint32_t column = 0;
    static uint8_t color = WHITE | (BLACK << 4);

    static uint16_t make_entry(char c, uint8_t entry_color) {
        return static_cast<uint16_t>(c) | (static_cast<uint16_t>(entry_color) << 8);
    }

    static void put_entry_at(char c, uint8_t entry_color, uint32_t x, uint32_t y) {
        video[y * WIDTH + x] = make_entry(c, entry_color);
    }

    static void scroll_if_needed() {
        if (row < HEIGHT) {
            return;
        }

        for (uint32_t y = 1; y < HEIGHT; ++y) {
            for (uint32_t x = 0; x < WIDTH; ++x) {
                video[(y - 1) * WIDTH + x] = video[y * WIDTH + x];
            }
        }

        for (uint32_t x = 0; x < WIDTH; ++x) {
            put_entry_at(' ', color, x, HEIGHT - 1);
        }

        row = HEIGHT - 1;
    }

    void init() {
        set_color(WHITE, BLACK);
        clear();
    }

    void clear() {
        for (uint32_t y = 0; y < HEIGHT; ++y) {
            for (uint32_t x = 0; x < WIDTH; ++x) {
                put_entry_at(' ', color, x, y);
            }
        }
        row = 0;
        column = 0;
    }

    void set_color(uint8_t foreground, uint8_t background) {
        color = foreground | (background << 4);
    }

    void put_char(char c) {
        if (c == '\b') {
            if (column > 0) {
                --column;
            } else if (row > 0) {
                --row;
                column = WIDTH - 1;
            }
            put_entry_at(' ', color, column, row);
            return;
        }

        if (c == '\n') {
            column = 0;
            ++row;
            scroll_if_needed();
            return;
        }

        if (c == '\r') {
            column = 0;
            return;
        }

        put_entry_at(c, color, column, row);
        ++column;

        if (column >= WIDTH) {
            column = 0;
            ++row;
            scroll_if_needed();
        }
    }

    void write(const char* text) {
        if (!text) {
            return;
        }
        while (*text) {
            put_char(*text++);
        }
    }

    void write_len(const char* text, uint32_t length) {
        if (!text) {
            return;
        }
        for (uint32_t i = 0; i < length; ++i) {
            put_char(text[i]);
        }
    }

    void write_line(const char* text) {
        write(text);
        put_char('\n');
    }

    void write_dec(uint32_t value) {
        char buffer[11];
        uint32_t index = 0;

        if (value == 0) {
            put_char('0');
            return;
        }

        while (value > 0 && index < sizeof(buffer)) {
            buffer[index++] = static_cast<char>('0' + (value % 10));
            value /= 10;
        }

        while (index > 0) {
            put_char(buffer[--index]);
        }
    }

    void write_hex(uint32_t value) {
        static const char* digits = "0123456789ABCDEF";
        write("0x");
        for (int shift = 28; shift >= 0; shift -= 4) {
            put_char(digits[(value >> shift) & 0xF]);
        }
    }
}
