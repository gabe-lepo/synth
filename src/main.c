#include "i2c/i2c.h"
#include "spi/spi.h"
#include "uart/uart.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  uart_init();
  spi_init();
  i2c_init();

  mcp4725_test();
  print_string("Done...");

  return 0;
}
