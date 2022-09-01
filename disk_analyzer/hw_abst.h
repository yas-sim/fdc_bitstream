#pragma once

#include <iostream>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <conio.h>
#else       // assuming linux
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace hw_abst {

int is_key_hit(void);
int get_key(void);

bool enable_escape_sequence(void);
void cls(void);
void csrpos(size_t x, size_t y);
void color(size_t col);

} // hw_abst
