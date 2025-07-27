#include "led/led.h"
#include "twi/twi.h"
#include "twi/util/twi_util.h"
#include "uart/uart.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

int main(void) {
  uart_init();
  twi_init();

  twi_scan();

  // printf("Setting DAC to 0\n");
  // mcp4725_write_dac_cmd_mode(0);
  // _delay_ms(3000);
  // printf("Setting DAC to 4095\n");
  // mcp4725_write_dac_cmd_mode(4095);
  mcp4725_triangle_wave();

  printf("Blinking led...\n");
  while (1) {
    led_on();
    _delay_ms(1000);
    led_off();
    _delay_ms(1000);
  }

  return 0;
}
