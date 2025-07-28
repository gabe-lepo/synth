#include "twi.h"
#include "../led/led.h"
#include "../util/debug.h"
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

  // printf("twi_init done, values:\n");
  // printf("\tF_CPU: %lu | TWBR_VALUE: %lu | TWPS1_VALUE: %d | TWPS0_VALUE:
  // %d\n", F_CPU, TWBR_VALUE, TWPS1_VALUE, TWPS0_VALUE);
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
