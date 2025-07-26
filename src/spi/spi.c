#include "spi.h"
#include "../uart/uart.h"
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

void temp(void) { print_string("temp"); }

void spi_setup(void) {
  // Set our PORTB pins as outs
  DDRB |= (1 << SS_PIN) | (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << DC_PIN) |
          (1 << RESET_PIN);

  // Set the PORTD pin as in for screen's BUSY signal
  // Not needed for the OLED ssd1306 display
  // DDRD &= ~(1 << BUSY_PIN);

  // Enable SPI | Set master | Set clock rate fck/16 (1MHz)
  // CPOL not set (leading edge = rising, trailing edge = falling)
  // CPHA not set (leading edge = sample, trailing edge = setup)
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);

  // Ensure both CPOL and CPHA are cleared
  SPCR &= ~((1 << CPOL) | (1 << CPHA));

  // Enable SPI | Set master | Set clock rate fck/4 (4MHz)
  // CPOL not set (leading edge = rising, trailing edge = falling)
  // CPHA not set (leading edge = sample, trailing edge = setup)
  // SPCR = (1 << SPE) | (1 << MSTR);

  // Double the SPI clock
  // SPSR |= (1 << SPI2X);

  // Dont double SCK rate
  if (SPSR & (1 << SPI2X)) {
    print_string("Found SPI2X bit, turning it off!");
    SPSR &= ~(1 << SPI2X);
  }

  // Ensure DORD is not set, so we send MSB first.
  if (SPCR & (1 << DORD)) {
    SPCR &= ~(1 << DORD);
  }
}

void spi_slave_select_low(void) { PORTB &= ~(1 << SS_PIN); }

void spi_slave_select_high(void) { PORTB |= (1 << SS_PIN); }

// Not needed for ssd1306 OLED
void spi_check_busy(void) {
  while (PIND & (1 << BUSY_PIN)) {
    _delay_ms(10);
  }
}

void spi_hw_reset(void) {
  PORTB &= ~(1 << RESET_PIN);
  _delay_ms(500);
  PORTB |= (1 << RESET_PIN);
  _delay_ms(500);
}

void spi_transmit(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)))
    ;
}

void spi_set_command_mode(void) {
  PORTB &= ~(1 << DC_PIN);
  // _delay_ms(10);
}

void spi_set_data_mode(void) {
  PORTB |= (1 << DC_PIN);
  // _delay_ms(10);
}

void spi_send_command(uint8_t command) {
  spi_set_command_mode();
  spi_slave_select_low();
  spi_transmit(command);
  spi_slave_select_high();
}

void spi_send_data(uint8_t data) {
  spi_set_data_mode();
  spi_slave_select_low();
  spi_transmit(data);
  spi_slave_select_high();
}

void test_dc_pin(void) {
  print_string("test_dc_pin");

  _delay_ms(3000);
  print_string("\tDC pin HIGH");
  PORTB |= (1 << DC_PIN);
  _delay_ms(5000);
  print_string("\tDC pin LOW");
  PORTB &= ~(1 << DC_PIN);

  print_string("test_dc_pin done");
}

void test_cs_pin(void) {
  print_string("test_cs_pin");

  _delay_ms(3000);
  print_string("\tCS pin HIGH");
  PORTB |= (1 << SS_PIN);
  _delay_ms(5000);
  print_string("\tCS pin LOW");
  PORTB &= ~(1 << SS_PIN);

  print_string("test_cs_pin done");
}

void test_reset_pin(void) {
  print_string("test_reset_pin");

  _delay_ms(3000);
  print_string("\tRESET pin LOW");
  PORTB &= ~(1 << RESET_PIN);
  _delay_ms(5000);
  print_string("\tRESET pin HIGH");
  PORTB |= (1 << RESET_PIN);

  print_string("test_reset_pin done");
}
