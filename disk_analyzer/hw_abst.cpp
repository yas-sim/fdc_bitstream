#include "hw_abst.h"

namespace hw_abst {

int is_key_hit(void) {
#ifdef _WIN32
     return kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
#endif
}

int get_key(void) {
#ifdef _WIN32
    return getch();
#else
    return getchar();
#endif
}


bool enable_escape_sequence(void) {
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

void cls(void) {
    std::cout << "\x1b[2J";
}

void csrpos(size_t x, size_t y) {
    std::cout << "\x1b[" << y << ";" << x << "H";
}

void color(size_t col) {
    col = col & 0x07;
    col = ((col & 0x02) ? 1:0) |
          ((col & 0x04) ? 2:0) |
          ((col & 0x01) ? 4:0);
    std::cout << "\x1b[" << 90 + col << "m";
}

} //hwabst
