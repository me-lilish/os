#include "user_program.h"
#include "core/ktypes.h"
#include "shell/shell.h"
#include "syscalls/syscall.h"

extern "C" void user_main() {
    Syscalls::write("\n[user] entered Ring 3 user mode\n");

    uint32_t pid = Syscalls::getpid();
    Syscalls::write("[user] getpid() = ");

    char digits[11];
    uint32_t index = 0;
    if (pid == 0) {
        digits[index++] = '0';
    } else {
        char reversed[11];
        uint32_t r = 0;
        while (pid > 0 && r < sizeof(reversed)) {
            reversed[r++] = static_cast<char>('0' + (pid % 10));
            pid /= 10;
        }
        while (r > 0) {
            digits[index++] = reversed[--r];
        }
    }
    digits[index++] = '\n';
    Syscalls::write(digits, index);

    uint32_t child_pid = Syscalls::fork();
    Syscalls::write("[user] fork() child pid = ");

    index = 0;
    if (child_pid == 0) {
        digits[index++] = '0';
    } else {
        char reversed[11];
        uint32_t r = 0;
        while (child_pid > 0 && r < sizeof(reversed)) {
            reversed[r++] = static_cast<char>('0' + (child_pid % 10));
            child_pid /= 10;
        }
        while (r > 0) {
            digits[index++] = reversed[--r];
        }
    }
    digits[index++] = '\n';
    Syscalls::write(digits, index);

    Syscalls::write("[user] demo complete; opening CLI...\n");

    Shell::run();
}
