#pragma once

#include <iostream>
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

namespace escseq {
    bool enable(void);
    void cls(void);
    void csrpos(size_t x, size_t y);
    void color(size_t col);
} // escseq
