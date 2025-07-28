#include "twi.h"
#include "../led/led.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

// Block until start/stop condition is done
static void twi_waitForComplete(void) {
  while (~TWCR & (1 << TWINT))
    ;
}

void twi_init(void) {
  TWBR = TWBR_VALUE;
  TWSR = (TWPS1_VALUE << TWPS1) | (TWPS0_VALUE << TWPS0);
  TWCR = (1 << TWEN);

  //clang-format off
  printf("twi_init done, values:\n");
  printf("\tF_CPU: %lu | TWBR_VALUE: %lu | TWPS1_VALUE: %d | TWPS0_VALUE: %d\n",
         F_CPU, TWBR_VALUE, TWPS1_VALUE, TWPS0_VALUE);
  //clang-format on
}

uint8_t twi_start(void) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  twi_waitForComplete();

  if (TW_STATUS != I2C_START_OK)
    return 1;
  else
    return 0;
}

void twi_stop(void) { TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); }

uint8_t twi_repStart(void) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

  twi_waitForComplete();

  if (TW_STATUS != I2C_REPEAT_START_OK)
    return 1;
  else
    return 0;
}

uint8_t twi_addressWrite(uint8_t addr) {
  TWDR = TWI_ADDRESS_W(addr);
  TWCR = (1 << TWINT) | (1 << TWEN);

  twi_waitForComplete();

  if (TW_STATUS != I2C_SLA_W_ACK)
    return 1;
  else
    return 0;
}

uint8_t twi_addressRead(uint8_t addr) {
  TWDR = TWI_ADDRESS_R(addr);
  TWCR = (1 << TWINT) | (1 << TWEN);

  twi_waitForComplete();

  if (TW_STATUS != I2C_SLA_R_ACK)
    return 1;
  else
    return 0;
}

uint8_t twi_write(uint8_t data) {
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);

  twi_waitForComplete();

  if (TW_STATUS != I2C_DATA_ACK)
    return 1;
  else
    return 0;
}

size_t twi_writeBurst(uint8_t *data, size_t len) {
  size_t i = len;

  while (i) {
    if (twi_write(*data++))
      break;
    i--;
  }

  return len - i;
}

uint8_t twi_readAck(uint8_t *data) {
  TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
  twi_waitForComplete();

  *data = TWDR;

  if (TW_STATUS != I2C_DATA_RX_ACK)
    return 1;
  else
    return 0;
}

size_t twi_readAckBurst(uint8_t *data, size_t len) {
  size_t i = len;

  while (i) {
    if (twi_readAck(data++))
      break;
    i--;
  }

  return len - i;
}

uint8_t twi_readNoAck(uint8_t *data) {
  TWCR = (1 << TWINT) | (1 << TWEN);
  twi_waitForComplete();

  *data = TWDR;

  if (TW_STATUS != I2C_DATA_RX_NACK)
    return 1;
  else
    return 0;
}

size_t twi_readNoAckBurst(uint8_t *data, size_t len) {
  size_t i = len;

  while (i) {
    if (twi_readNoAck(data++))
      break;
    i--;
  }

  return len - i;
}

uint8_t twi_writeToSlave(uint8_t address, uint8_t *data, size_t len) {
  if (twi_start())
    return 1;
  if (twi_addressWrite(address)) {
    twi_stop();
    return 1;
  }

  if (twi_writeBurst(data, len) != len) {
    twi_stop();
    return 1;
  }

  twi_stop();

  return 0;
}

uint8_t twi_readFromSlave(uint8_t address, uint8_t *data, size_t len) {
  if (twi_start())
    return 1;
  if (twi_addressRead(address)) {
    twi_stop();
    return 1;
  }

  if (len > 1 && twi_readAckBurst(data, len - 1) != len - 1) {
    twi_stop();
    return 1;
  }
  if (len > 0 && twi_readNoAck(&data[len - 1])) {
    twi_stop();
    return 1;
  }

  twi_stop();

  return 0;
}

uint8_t twi_writeToSlaveRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                 size_t len) {
  if (twi_start())
    return 1;
  if (twi_addressWrite(address)) {
    twi_stop();
    return 1;
  }

  if (twi_write(reg)) {
    twi_stop();
    return 1;
  }
  if (twi_writeBurst(data, len) != len) {
    twi_stop();
    return 1;
  }

  twi_stop();

  return 0;
}

uint8_t twi_readFromSlaveRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                  size_t len) {
  if (twi_start())
    return 1;
  if (twi_addressWrite(address)) {
    twi_stop();
    return 1;
  }
  if (twi_write(reg)) {
    twi_stop();
    return 1;
  }

  if (twi_repStart())
    return 1;
  if (twi_addressRead(address)) {
    twi_stop();
    return 1;
  }

  if (len > 1 && twi_readAckBurst(data, len - 1) != len - 1) {
    twi_stop();
    return 1;
  }
  if (len > 0 && twi_readNoAck(&data[len - 1])) {
    twi_stop();
    return 1;
  }

  twi_stop();

  return 0;
}

// mcp4725 funcs
uint8_t mcp4725_write_dac(uint16_t value) {
  printf("Starting write_dac with value: 0x%04X\n", value);
  // Ensure 12-bit value
  value &= 0x0FFF;
  printf("Value after 12-bit assert: 0x%04X\n", value);

  // Start TWI
  if (twi_start()) {
    printf("TWI start failed\n");
    return 1;
  }

  // Send device address with write bit
  printf("Sending address write: 0x%02X\n", MCP4725_ADDR);
  if (twi_addressWrite(MCP4725_ADDR)) {
    printf("TWI address write failed\n");
    twi_stop();
    return 1;
  }

  // Fast mode: 2 data bytes (no command byte)
  // Byte 1: upper 8 bits of value
  uint8_t byte1 = (value >> 4) & 0xFF;
  printf("Sending upper 8 bits of value: 0x%02X (%d)\n", byte1, byte1);

  if (twi_write(byte1)) {
    printf("TWI byte 1 failed\n");
    twi_stop();
    return 1;
  }

  // Byte 2: Lower 4 bits shifted left
  uint8_t byte2 = (value & 0x0F) << 4;
  byte2 |= 0x00; // No power down bits
  printf(
      "Sending lower 4 bits shifted left with no power off bits: 0x%02X (%d)\n",
      byte2, byte2);

  if (twi_write(byte2)) {
    printf("TWI byte 2 failed\n");
    twi_stop();
    return 1;
  }

  twi_stop();
  return 0;
}

uint8_t mcp4725_write_dac_cmd_mode(uint16_t value) {
  led_on();

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
  led_off();
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
      // _delay_us(10);
    }

    // Falling edge
    printf("Falling edge!\r\n");
    for (counter = MAX_VALUE; counter > 0; counter--) {
      if (mcp4725_write_dac_cmd_mode(counter)) {
        printf("Failed writing to DAC for falling edge, counter: %u\n",
               counter);
        return;
      }
      // _delay_us(10);
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
