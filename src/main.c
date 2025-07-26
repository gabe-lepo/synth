#include "spi/spi.h"
#include "uart/uart.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  uart_init();
  spi_setup();

  print_string("Done...");

  return 0;
}
