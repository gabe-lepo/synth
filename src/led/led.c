#include "led.h"
#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PB4 // physical D12 pin

void led_init(void) {
  DDRB |= (1 << LED_PIN);
  led_off();
}

void led_on(void) { PORTB |= (1 << LED_PIN); }

void led_off(void) { PORTB &= ~(1 << LED_PIN); }

void led_blink(uint16_t num_blink) {
  for (uint16_t counter = 0; counter < num_blink; counter++) {
    led_on();
    _delay_ms(200);
    led_off();
    _delay_ms(200);
  }
}
