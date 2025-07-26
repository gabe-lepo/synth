#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);
void uart_transmit(unsigned char data);
void print_string(const char *str);
void print_hex(uint8_t value);

#endif
