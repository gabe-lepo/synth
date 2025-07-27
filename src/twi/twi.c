#include "twi.h"
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
  if (twi_write((value >> 4) & 0xFF)) {
    printf("TWI fast byte 1 failed\n");
    twi_stop();
    return 1;
  }

  // Byte 2: Lower 4 bits shifted left + b00 bits for
  // no power-down state (normal operation)
  if (twi_write(((value & 0x0F) << 4) | 0x00)) {
    printf("TWI fast byte 2 failed\n");
    twi_stop();
    return 1;
  }

  twi_stop();
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
      if (mcp4725_write_dac(counter)) {
        printf("Failed writing to DAC for rising edge, counter: %u\n", counter);
        return;
      }
      if (counter % 100 == 0) {
        printf("%u\r", counter);
      }
      _delay_ms(1);
    }

    // Falling edge
    printf("Falling edge!\r\n");
    for (counter = MAX_VALUE; counter > 0; counter--) {
      if (mcp4725_write_dac(counter)) {
        printf("Failed writing to DAC for falling edge, counter: %u\n",
               counter);
        return;
      }
      if (counter % 100 == 0) {
        printf("%u\r", counter);
      }
      _delay_ms(1);
    }
  }
}

void mcp4725_square_wave(void) {
  printf("Generating square wave...\n");

  while (1) {
    printf("Low\n");
    if (mcp4725_write_dac(0)) {
      printf("Failed writing to DAC for low side\n");
      return;
    }
    _delay_ms(2000);

    printf("High\n");
    if (mcp4725_write_dac(4095)) {
      printf("Failed writing to DAC for low side\n");
      return;
    }
    _delay_ms(2000);
  }
}
