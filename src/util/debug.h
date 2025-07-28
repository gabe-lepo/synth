#ifndef DEBUG_H
#define DEBUG_H

// Silly clang things to avoid lsp warnings
#ifdef __clang__
#include <stdio.h>
#else
// Keep printf strings in flash
#include <avr/pgmspace.h>
#include <stdio.h>
#define printf(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
#endif

#endif
