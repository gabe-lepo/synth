#ifndef TWI_H
#define TWI_H

#include <avr/io.h>
#include <stdint.h>

#define F_CPU 16000000UL
#define TWI_FREQ_400K 400000
#define TWI_FREQ_100K 100000

#if (F_CPU / TWI_FREQ_400K - 16) / (2 * 1) >= 10 &&                            \
    (F_CPU / TWI_FREQ_400K - 16) / (2 * 1) <= 0xFF
#define TWI_PRESCALER 1
#define TWPS0_VALUE 0
#define TWPS1_VALUE 0
#elif (F_CPU / TWI_FREQ_400K - 16) / (2 * 4) >= 10 &&                          \
    (F_CPU / TWI_FREQ_400K - 16) / (2 * 4) <= 0xFF
#define TWI_PRESCALER 4
#define TWPS0_VALUE 1
#define TWPS1_VALUE 0
#elif (F_CPU / TWI_FREQ_400K - 16) / (2 * 16) >= 10 &&                         \
    (F_CPU / TWI_FREQ_400K - 16) / (2 * 16) <= 0xFF
#define TWI_PRESCALER 16
#define TWPS0_VALUE 0
#define TWPS1_VALUE 1
#elif (F_CPU / TWI_FREQ_400K - 16) / (2 * 64) >= 10 &&                         \
    (F_CPU / TWI_FREQ_400K - 16) / (2 * 64) <= 0xFF
#define TWI_PRESCALER 64
#define TWPS0_VALUE 1
#define TWPS1_VALUE 1
#else
#error "TWI_FREQ_400K too low!"
#endif

#define TWBR_VALUE ((F_CPU / TWI_FREQ_400K - 16) / (2 * TWI_PRESCALER))

// Mask TWI slave addressing byte with given id and write/read intend.
#define TWI_ADDRESS_W(id) (((id) << 1) & ~0x01) // write
#define TWI_ADDRESS_R(id) (((id) << 1) | 0x01)  // read

#define TW_STATUS_MASK                                                         \
  (_BV(TWS7) | _BV(TWS6) | _BV(TWS5) | _BV(TWS4) | _BV(TWS3))
#define TW_STATUS (TWSR & TW_STATUS_MASK)

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

// twi
void twi_init(void);
uint8_t twi_start(void);
void twi_stop(void);
uint8_t twi_addressWrite(uint8_t addr);

// mcp4725 funcs
uint8_t mcp4725_write_dac(uint16_t value);
uint8_t mcp4725_write_dac_cmd_mode(uint16_t value);
void mcp4725_triangle_wave(void);
void mcp4725_square_wave(void);

#endif
