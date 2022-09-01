#pragma once

#include <iostream>
#include <stdio.h>
#include <io.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace escseq {
    bool enable(void);
    void cls(void);
    void csrpos(size_t x, size_t y);
    void color(size_t col);
} // escseq
