#include "mcp4725.h"
#include "../led/led.h"
#include "../twi/twi.h"
#include "../util/debug.h"
#include <stdio.h>
#include <util/delay.h>

uint8_t mcp4725_write_dac(uint16_t value, uint8_t cmd_byte, uint8_t eeprom) {
  // printf("%d\n", value);
  value &= 0x0FFF;

  if (twi_start()) {
    printf("TWI start failed\n");
    return 1;
  }

  if (twi_addressWrite(MCP4725_ADDR)) {
    printf("TWI address write failed\n");
    twi_stop();
    return 1;
  }

  if (cmd_byte) {
    if (eeprom) {
      if (twi_write(MCP4725_CMD_WRITEDACEEPROM)) {
        printf("TWI command byte failed\n");
        twi_stop();
        return 1;
      }
    } else {
      if (twi_write(MCP4725_CMD_WRITEDAC)) {
        printf("TWI command byte failed\n");
        twi_stop();
        return 1;
      }
    }
  }

  uint8_t byte1 = (value >> 4) & 0xFF;
  if (twi_write(byte1)) {
    printf("TWI byte 1 failed\n");
    twi_stop();
    return 1;
  }

  uint8_t byte2 = (value & 0x0F) << 4;
  byte2 |= 0x00; // No power down bits
  if (twi_write(byte2)) {
    printf("TWI byte 2 failed\n");
    twi_stop();
    return 1;
  }

  twi_stop();
  _delay_ms(1);
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
      if (mcp4725_write_dac(counter, 1, 0)) {
        printf("Failed writing to DAC for rising edge, counter: %u\n", counter);
        twi_stop();
        return;
      }
    }

    // Falling edge
    printf("Falling edge!\r\n");
    for (counter = MAX_VALUE; counter > 0; counter--) {
      if (mcp4725_write_dac(counter, 1, 0)) {
        printf("Failed writing to DAC for falling edge, counter: %u\n",
               counter);
        twi_stop();
        return;
      }
    }

    twi_stop();
  }
}

void mcp4725_square_wave(void) {
  printf("Generating square wave...\n");

  while (1) {
    printf("Low\n");
    if (mcp4725_write_dac(0, 1, 0)) {
      printf("Failed writing to DAC for low side\n");
      return;
    }
    _delay_ms(3000);

    printf("High\n");
    if (mcp4725_write_dac(4095, 1, 0)) {
      printf("Failed writing to DAC for low side\n");
      return;
    }
    _delay_ms(3000);
  }
}
