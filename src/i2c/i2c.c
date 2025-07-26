#include "i2c.h"
#include "../uart/uart.h"
#include <stdint.h>
#include <util/delay.h>

// i2c funcs
void wait_twint(void) {
  while (!(TWCR & (1 << TWINT)))
    ;
  return;
}

uint8_t i2c_get_status(void) { return (TWSR & 0xF8); }

void i2c_init(void) {
  // Set SCL freq to 100kHz with 16MHz mcu clock
  // SCL freq = F_CPU / (16 + 2 * TWBR * 4^TWPS)
  // For 100kHz: TWBR = 72, TWPS = 0
  TWBR = 72;
  TWSR = 0x00;

  TWCR = (1 << TWEN);

  print_string("I2C initialized at 100kHz");
  return;
}

uint8_t i2c_start(void) {
  // Start condition
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

  wait_twint();
  return i2c_get_status();
}

uint8_t i2c_write(uint8_t data) {
  // Load data into register
  TWDR = data;

  // Clear TWINT to start transmission
  TWCR = (1 << TWINT) | (1 << TWEN);

  wait_twint();
  return i2c_get_status();
}

uint8_t i2c_read(uint8_t ack) {
  if (ack) {
    // Enable ACK and clear TWINT
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  } else {
    // Clear TWINT without ACK
    TWCR = (1 << TWINT) | (1 << TWEN);
  }

  wait_twint();
  return TWDR;
}

void i2c_stop(void) {
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
  while (!(TWCR & (1 << TWSTO)))
    ;
  return;
}

// mcp4725 funcs
uint8_t mcp4725_write_dac(uint16_t value, uint8_t write_eeprom) {
  uint8_t status;
  uint8_t command;

  // Ensure 12-bit value
  value &= 0x0FFF;

  // Choose command from input arg
  command = write_eeprom ? MCP4725_CMD_WRITEDACEEPROM : MCP4725_CMD_WRITEDAC;

  // Start cond
  status = i2c_start();
  if (status != I2C_START_OK) {
    print_string("I2C START failed with status:");
    print_hex(status);
    i2c_stop();
    return 1;
  }

  // Send device addr with write bit
  status = i2c_write((MCP4725_ADDR << 1) | 0);
  if (status != I2C_SLA_W_ACK) {
    print_string("I2C device address failed with status:");
    print_hex(status);
    i2c_stop();
    return 1;
  }

  // Send command and upper 4-bits of data
  status = i2c_write(command | ((value >> 8) & 0x0F));
  if (status != I2C_DATA_ACK) {
    print_string("I2C command byte failed with status:");
    print_hex(status);
    i2c_stop();
    return 1;
  }

  // Send lower 8-bits of data
  status = i2c_write(value & 0xFF);
  if (status != I2C_DATA_ACK) {
    print_string("I2C data byte failed with status:");
    print_hex(status);
    i2c_stop();
    return 1;
  }

  i2c_stop();

  // eeprom write takes ~50ms
  if (write_eeprom) {
    _delay_ms(60);
  }

  return 0;
}

void mcp4725_test(void) {
  print_string("Testing MCP4725 DAC...");

  uint16_t test_values[] = {0, 1024, 2048, 3072, 4095};
  uint8_t values_size = sizeof(test_values) / sizeof(test_values[0]);

  for (uint8_t i = 0; i < values_size; i++) {
    print_string("Setting DAC to:");
    print_hex((test_values[i] >> 8) & 0xFF);
    print_hex(test_values[i] & 0xFF);

    if (mcp4725_write_dac(test_values[i], 0)) {
      print_string("Failed");
    } else {
      print_string("Success");
    }

    _delay_ms(1000);
  }

  print_string("Testing MCP4725 DAC...");
  return;
}
