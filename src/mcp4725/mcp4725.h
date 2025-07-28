#ifndef MCP4725_H
#define MCP4725_H

#include <stdint.h>

uint8_t mcp4725_write_dac(uint16_t value, uint8_t cmd_byte, uint8_t eeprom);
void mcp4725_triangle_wave(void);
void mcp4725_square_wave(void);

#endif
