#include "uart.h"
#include <avr/io.h>
#include <stdio.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MY_UBRR ((F_CPU / 16 / BAUD) - 1)

// Static func for stdio integration
static int uart_putchar(char c, FILE *stream);

// Create file stream for uart
static FILE uart_output =
    FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void uart_init(void) {
  // Set baud rate
  UBRR0H = (unsigned char)(MY_UBRR >> 8);
  UBRR0L = (unsigned char)(MY_UBRR);

  // Enable transmitter
  UCSR0B = (1 << TXEN0);

  // Set frame format: 8 data bits, 1 stop
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

  // Redirect stdout to UART
  stdout = &uart_output;
}

void uart_transmit(unsigned char data) {
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0)))
    ;

  UDR0 = data;
}

static int uart_putchar(char c, FILE *stream) {
  // Avoid compiler warning
  (void)stream;

  // Convert '\n' to '\r\n'
  if (c == '\n') {
    uart_transmit('\r');
  }
  uart_transmit(c);
  return 0;
}
