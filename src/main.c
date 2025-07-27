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

  printf("Blinking led...\n");
  while (1) {
    led_on();
    _delay_ms(1000);
    led_off();
    _delay_ms(1000);
  }

  return 0;
}
