#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <stdint.h>

// I2C pins on the atmega328p
#define SDA_PIN PC4 // physical A4
#define SCL_PIN PC5 // physical A5

// I2C Status codes
#define I2C_START_OK 0x08
#define I2C_REPEAT_START_OK 0x10
#define I2C_SLA_W_ACK 0x18
#define I2C_SLA_W_NACK 0x20
#define I2C_DATA_ACK 0x28
#define I2C_DATA_NACK 0x30
#define I2C_SLA_R_ACK 0x40
#define I2C_SLA_R_NACK 0x48
#define I2C_DATA_RX_ACK 0x50
#define I2C_DATA_RX_NACK 0x58

// MCP4725
#define MCP4725_ADDR 0x62
#define MCP4725_CMD_WRITEDAC 0x40
#define MCP4725_CMD_WRITEDACEEPROM 0X60

// i2c funcs
void i2c_init(void);
uint8_t i2c_start(void);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read(uint8_t ack);
void i2c_stop(void);
uint8_t i2c_get_status(void);

// mcp4725 funcs
uint8_t mcp4725_write_dac(uint16_t value, uint8_t write_eeprom);
void mcp4725_test(void);

#endif
