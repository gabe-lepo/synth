#include "mcp4725.h"
#include "../led/led.h"
#include "../twi/twi.h"
#include "../util/debug.h"
#include <util/delay.h>

uint8_t mcp4725_write_dac(uint16_t value) {
  // Ensure 12-bit value
  value &= 0x0FFF;

  // Start TWI
  if (twi_start()) {
    printf("TWI start failed\n");
    return 1;
  }

  // Send device address with write bit
  if (twi_addressWrite(MCP4725_ADDR)) {
    printf("TWI address write failed\n");
    twi_stop();
    return 1;
  }

  // Fast mode: 2 data bytes (no command byte)
  // Byte 1: upper 8 bits of value
  uint8_t byte1 = (value >> 4) & 0xFF;
  if (twi_write(byte1)) {
    printf("TWI byte 1 failed\n");
    twi_stop();
    return 1;
  }

  // Byte 2: Lower 4 bits shifted left
  uint8_t byte2 = (value & 0x0F) << 4;
  byte2 |= 0x00; // No power down bits
  if (twi_write(byte2)) {
    printf("TWI byte 2 failed\n");
    twi_stop();
    return 1;
  }

  twi_stop();
  return 0;
}

uint8_t mcp4725_write_dac_cmd_mode(uint16_t value) {
  // led_on();

  value &= 0x0FFF;
  printf("%d\n", value);

  if (twi_start()) {
    printf("TWI start failed\n");
    return 1;
  }

  if (twi_addressWrite(MCP4725_ADDR)) {
    printf("TWI address write failed\n");
    twi_stop();
    return 1;
  }

  // Command mode, 3 bytes
  // Byte 1: Command
  if (twi_write(MCP4725_CMD_WRITEDAC)) {
    printf("TWI command byte failed\n");
    twi_stop();
    return 1;
  }

  // Byte 2: Upper 8 bits
  uint8_t byte2 = (value >> 4) & 0xFF;
  // printf("Sending byte 2: 0x%02X\n", byte2);
  if (twi_write(byte2)) {
    printf("TWI byte 2 failed\n");
    twi_stop();
    return 1;
  }

  // Byte 3: Lower 4 bits
  uint8_t byte3 = (value & 0x0F) << 4;
  // printf("Sending byte 3: 0x%02X\n", byte3);
  if (twi_write(byte3)) {
    printf("TWI byte 3 failed\n");
    twi_stop();
    return 1;
  }

  twi_stop();
  _delay_ms(1);
  // led_off();
  return 0;
}

void mcp4725_triangle_wave(void) {
  const uint16_t MAX_VALUE = 4095;
  uint16_t counter;

  printf("Generating triangle wave...\n");

  while (1) {
    // Rising edge
    printf("Rising edge!\n");
    for (counter = 0; counter <= MAX_VALUE; counter++) {
      if (mcp4725_write_dac_cmd_mode(counter)) {
        printf("Failed writing to DAC for rising edge, counter: %u\n", counter);
        return;
      }
      if (counter % 100 == 0) {
        led_on();
        _delay_ms(5);
        led_off();
      }
    }

    // Falling edge
    printf("Falling edge!\r\n");
    for (counter = MAX_VALUE; counter > 0; counter--) {
      if (mcp4725_write_dac_cmd_mode(counter)) {
        printf("Failed writing to DAC for falling edge, counter: %u\n",
               counter);
        return;
      }
      if (counter % 100 == 0) {
        led_on();
        _delay_ms(5);
        led_off();
      }
    }
  }
}

void mcp4725_square_wave(void) {
  printf("Generating square wave...\n");

  while (1) {
    printf("Low\n");
    if (mcp4725_write_dac_cmd_mode(0)) {
      printf("Failed writing to DAC for low side\n");
      return;
    }
    _delay_ms(3000);

    printf("High\n");
    if (mcp4725_write_dac_cmd_mode(4095)) {
      printf("Failed writing to DAC for low side\n");
      return;
    }
    _delay_ms(3000);
  }
}
