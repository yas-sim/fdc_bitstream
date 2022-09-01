#include "esc_seq.h"

bool escseq::enable(void) {
#ifdef _WIN32      // enable escape sequence (Windows)
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);;
    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode)) {
        return false;
    }
    if (!SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        return false;
    }
#endif
    return true;
}

void escseq::cls(void) {
    std::cout << "\x1b[2J";
}

void escseq::csrpos(size_t x, size_t y) {
    std::cout << "\x1b[" << y << ";" << x << "H";
}

void escseq::color(size_t col) {
    col = col & 0x07;
    col = ((col & 0x02) ? 1:0) |
          ((col & 0x04) ? 2:0) |
          ((col & 0x01) ? 4:0);
    std::cout << "\x1b[" << 90 + col << "m";
}
