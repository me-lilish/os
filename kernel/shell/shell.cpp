#include "shell.h"
#include "../core/ktypes.h"
#include "../core/math.h"
#include "../drivers/keyboard.h"
#include "../drivers/vga.h"
#include "../syscalls/syscall.h"

namespace {
    constexpr uint32_t LINE_MAX = 96;

    char lower_char(char c) {
        if (c >= 'A' && c <= 'Z') {
            return static_cast<char>(c - 'A' + 'a');
        }
        return c;
    }

    bool is_space(char c) {
        return c == ' ' || c == '\t';
    }

    const char* skip_spaces(const char* text) {
        while (*text && is_space(*text)) {
            ++text;
        }
        return text;
    }

    bool equals_ci(const char* a, const char* b) {
        a = skip_spaces(a);
        while (*a && *b) {
            if (lower_char(*a) != lower_char(*b)) {
                return false;
            }
            ++a;
            ++b;
        }
        a = skip_spaces(a);
        return *a == 0 && *b == 0;
    }

    bool starts_with_ci(const char* text, const char* prefix) {
        text = skip_spaces(text);
        while (*prefix) {
            if (lower_char(*text) != lower_char(*prefix)) {
                return false;
            }
            ++text;
            ++prefix;
        }
        return true;
    }

    bool match_name(const char*& text, const char* name) {
        text = skip_spaces(text);
        while (*name) {
            if (lower_char(*text) != lower_char(*name)) {
                return false;
            }
            ++text;
            ++name;
        }
        return true;
    }

    uint32_t text_length(const char* text) {
        uint32_t length = 0;
        while (text && text[length]) {
            ++length;
        }
        return length;
    }

    void write_int(int value) {
        if (value < 0) {
            VGA::put_char('-');
            value = -value;
        }
        VGA::write_dec(static_cast<uint32_t>(value));
    }

    void read_line(char* buffer, uint32_t max_length) {
        uint32_t length = 0;
        while (true) {
            char c = Keyboard::get_char();

            if (c == '\n') {
                VGA::put_char('\n');
                buffer[length] = 0;
                return;
            }

            if (c == '\b') {
                if (length > 0) {
                    --length;
                    VGA::put_char('\b');
                }
                continue;
            }

            if (c >= ' ' && c <= '~' && length + 1 < max_length) {
                buffer[length++] = c;
                VGA::put_char(c);
            }
        }
    }

    bool parse_int(const char*& text, int& value) {
        text = skip_spaces(text);

        int sign = 1;
        if (*text == '-') {
            sign = -1;
            ++text;
        } else if (*text == '+') {
            ++text;
        }

        if (*text < '0' || *text > '9') {
            return false;
        }

        int number = 0;
        while (*text >= '0' && *text <= '9') {
            number = Math::add(Math::multiply(number, 10), *text - '0');
            ++text;
        }

        value = Math::multiply(number, sign);
        text = skip_spaces(text);
        return true;
    }

    bool parse_binary_expression(const char* text, int& result, bool& divide_by_zero) {
        divide_by_zero = false;
        int left = 0;
        int right = 0;
        if (!parse_int(text, left)) {
            return false;
        }

        char op = *text;
        if (op != '+' && op != '-' && op != '*' && op != '/' && op != '%') {
            return false;
        }
        ++text;

        if (!parse_int(text, right)) {
            return false;
        }

        if (*skip_spaces(text) != 0) {
            return false;
        }

        if ((op == '/' || op == '%') && right == 0) {
            divide_by_zero = true;
            return true;
        }

        if (op == '+') {
            result = Math::add(left, right);
        } else if (op == '-') {
            result = Math::subtract(left, right);
        } else if (op == '*') {
            result = Math::multiply(left, right);
        } else if (op == '/') {
            result = Math::divide(left, right);
        } else {
            result = Math::modulo(left, right);
        }

        return true;
    }

    bool parse_two_arg_function(const char* text, const char* name, int& left, int& right) {
        if (!match_name(text, name)) {
            return false;
        }

        text = skip_spaces(text);
        if (*text != '(') {
            return false;
        }
        ++text;

        if (!parse_int(text, left)) {
            return false;
        }

        if (*text != ',') {
            return false;
        }
        ++text;

        if (!parse_int(text, right)) {
            return false;
        }

        return *skip_spaces(text) == ')';
    }

    bool parse_round_function(const char* text, int& result) {
        if (!match_name(text, "round")) {
            return false;
        }

        text = skip_spaces(text);
        if (*text != '(') {
            return false;
        }
        ++text;
        text = skip_spaces(text);

        int sign = 1;
        if (*text == '-') {
            sign = -1;
            ++text;
        } else if (*text == '+') {
            ++text;
        }

        if (*text < '0' || *text > '9') {
            return false;
        }

        int whole = 0;
        while (*text >= '0' && *text <= '9') {
            whole = Math::add(Math::multiply(whole, 10), *text - '0');
            ++text;
        }

        int scale = 1;
        int fraction = 0;
        if (*text == '.') {
            ++text;
            if (*text < '0' || *text > '9') {
                return false;
            }
            while (*text >= '0' && *text <= '9' && scale < 10000) {
                fraction = Math::add(Math::multiply(fraction, 10), *text - '0');
                scale = Math::multiply(scale, 10);
                ++text;
            }
            while (*text >= '0' && *text <= '9') {
                ++text;
            }
        }

        if (*skip_spaces(text) != ')') {
            return false;
        }

        int scaled = Math::add(Math::multiply(whole, scale), fraction);
        result = Math::round_scaled(Math::multiply(scaled, sign), scale);
        return true;
    }

    bool parse_function_expression(const char* text, int& result, bool& divide_by_zero) {
        int left = 0;
        int right = 0;
        divide_by_zero = false;

        if (parse_two_arg_function(text, "add", left, right)) {
            result = Math::add(left, right);
            return true;
        }
        if (parse_two_arg_function(text, "sub", left, right)) {
            result = Math::subtract(left, right);
            return true;
        }
        if (parse_two_arg_function(text, "mul", left, right)) {
            result = Math::multiply(left, right);
            return true;
        }
        if (parse_two_arg_function(text, "div", left, right)) {
            if (right == 0) {
                divide_by_zero = true;
            } else {
                result = Math::divide(left, right);
            }
            return true;
        }
        if (parse_two_arg_function(text, "mod", left, right)) {
            if (right == 0) {
                divide_by_zero = true;
            } else {
                result = Math::modulo(left, right);
            }
            return true;
        }
        return parse_round_function(text, result);
    }

    void print_calculator_help() {
        VGA::write_line("Calculator examples:");
        VGA::write_line("  2+3        9-4        6*7        20/5        20%6");
        VGA::write_line("  add(2,3)   sub(9,4)   mul(6,7)   div(20,5)");
        VGA::write_line("  round(2.6)");
        VGA::write_line("  exit");
    }

    void run_calculator() {
        char line[LINE_MAX];

        VGA::clear();
        VGA::write_line("MiniOS calculator opened with open()");
        print_calculator_help();

        while (true) {
            VGA::write("calc> ");
            read_line(line, sizeof(line));

            if (equals_ci(line, "")) {
                continue;
            }
            if (equals_ci(line, "exit") || equals_ci(line, "quit") || equals_ci(line, "back")) {
                VGA::clear();
                return;
            }
            if (equals_ci(line, "help")) {
                print_calculator_help();
                continue;
            }
            if (equals_ci(line, "clear")) {
                VGA::clear();
                continue;
            }

            int result = 0;
            bool divide_by_zero = false;
            if (parse_binary_expression(line, result, divide_by_zero) ||
                parse_function_expression(line, result, divide_by_zero)) {
                if (divide_by_zero) {
                    VGA::write_line("error: division by zero");
                } else {
                    VGA::write("= ");
                    write_int(result);
                    VGA::put_char('\n');
                }
            } else {
                VGA::write_line("unknown calculator input; type help");
            }
        }
    }

    void print_shell_help() {
        VGA::write_line("Commands:");
        VGA::write_line("  getpid()        show the current process id syscall");
        VGA::write_line("  fork()          create a child PCB with three pages");
        VGA::write_line("  write(text)     write text through the write syscall");
        VGA::write_line("  open()          open the calculator");
        VGA::write_line("  clear           clear the screen");
        VGA::write_line("  help            show this list");
    }

    void print_shell_banner() {
        VGA::write_line("MiniOS CLI running in Ring 3 user mode");
        VGA::write_line("Type help for commands. Type open() for calculator.");
    }

    void run_write_command(const char* line) {
        const char* text = line;
        if (!match_name(text, "write")) {
            return;
        }

        text = skip_spaces(text);
        if (*text == 0) {
            Syscalls::write("[shell] write syscall from CLI\n");
            return;
        }

        if (*text == '(') {
            ++text;
            text = skip_spaces(text);
            if (*text == '"') {
                ++text;
                const char* end = text;
                while (*end && *end != '"') {
                    ++end;
                }
                Syscalls::write(text, static_cast<uint32_t>(end - text));
                Syscalls::write("\n");
                return;
            }

            const char* end = text;
            while (*end && *end != ')') {
                ++end;
            }
            Syscalls::write(text, static_cast<uint32_t>(end - text));
            Syscalls::write("\n");
            return;
        }

        Syscalls::write(text, text_length(text));
        Syscalls::write("\n");
    }

    void run_shell_command(const char* line) {
        if (equals_ci(line, "")) {
            return;
        }

        if (equals_ci(line, "help")) {
            print_shell_help();
        } else if (equals_ci(line, "clear")) {
            VGA::clear();
            print_shell_banner();
        } else if (equals_ci(line, "getpid()") || equals_ci(line, "getpid")) {
            VGA::write("getpid() = ");
            VGA::write_dec(Syscalls::getpid());
            VGA::put_char('\n');
        } else if (equals_ci(line, "fork()") || equals_ci(line, "fork")) {
            uint32_t child_pid = Syscalls::fork();
            if (child_pid == 0) {
                VGA::write_line("fork() failed: no PCB slots left");
            } else {
                VGA::write("fork() child pid = ");
                VGA::write_dec(child_pid);
                VGA::put_char('\n');
            }
        } else if (starts_with_ci(line, "write")) {
            run_write_command(line);
        } else if (equals_ci(line, "open()") || equals_ci(line, "open")) {
            run_calculator();
            print_shell_banner();
        } else {
            VGA::write_line("unknown command; type help");
        }
    }
}

namespace Shell {
    void run() {
        char line[LINE_MAX];

        VGA::clear();
        print_shell_banner();

        while (true) {
            VGA::write("os> ");
            read_line(line, sizeof(line));
            run_shell_command(line);
        }
    }
}
