#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <stdint.h>

#define SS_PIN PB2
#define MOSI_PIN PB3
#define MISO_PIN PB4
#define SCK_PIN PB5
#define DC_PIN PB1
#define RESET_PIN PB0
#define BUSY_PIN PD2

void spi_setup(void);
void spi_slave_select_low(void);
void spi_slave_select_high(void);
void spi_check_busy(void);
void spi_hw_reset(void);
void spi_set_command_mode(void);
void spi_set_data_mode(void);
void spi_transmit(uint8_t);
void spi_send_command(uint8_t command);
void spi_send_data(uint8_t data);

void test_dc_pin(void);
void test_cs_pin(void);
void test_reset_pin(void);

#endif
