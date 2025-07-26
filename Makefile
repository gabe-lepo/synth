# Target
MCU = atmega328p

# Clock freq
F_CPU = 16000000UL # 16 MHz

# Optimization level
OPT = -Os

# Directories
DIR_SPI = src/spi
DIR_UART = src/uart
DIR_MAIN = src/main
DIR_TEST = src/test
DIR_BUILD = build

# Object files for main program
OBJ = $(DIR_BUILD)/main.o \
      $(DIR_BUILD)/spi.o \
      $(DIR_BUILD)/uart.o \

# Output files
ELF = $(DIR_BUILD)/main.elf
HEX = $(DIR_BUILD)/main.hex

# Comm settings
FLASH_PORT = /dev/cu.usbserial-AQ04QNJZ
FLASH_BAUD = 115200
COMM_PORT = /dev/tty.usbserial-AQ04QNJZ
COMM_BAUD = 9600

# Compiler/linker flags
CFLAGS = -mmcu=$(MCU) \
         -DF_CPU=$(F_CPU) \
         $(OPT) \
         -I$(DIR_SPI) \
         -I$(DIR_UART) \
         -Wall -Wextra
LDFLAGS = -mmcu=$(MCU)

# Default target
all: clean build upload connect

# Build target
build: $(HEX)

# Object file compilation
$(DIR_BUILD)/main.o: $(DIR_MAIN)/main_oled.c | $(DIR_BUILD)
	avr-gcc $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/spi.o: $(DIR_SPI)/spi.c | $(DIR_BUILD)
	avr-gcc $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/uart.o: $(DIR_UART)/uart.c | $(DIR_BUILD)
	avr-gcc $(CFLAGS) -c $< -o $@

# Link
$(ELF): $(OBJ)
	avr-gcc $(LDFLAGS) -o $(ELF) $(OBJ)

# ELF to HEX
$(HEX): $(ELF)
	avr-objcopy -O ihex -R .eeprom $(ELF) $(HEX)

# Upload target
upload: $(HEX)
	avrdude -v -patmega328p -carduino -P$(FLASH_PORT) -b $(FLASH_BAUD) -D -Uflash:w:$(HEX):i

# Serial connection
connect:
	picocom $(COMM_PORT) -b $(COMM_BAUD)

# Clean target
clean:
	rm -rf $(DIR_BUILD)/*

# Phony targets
.PHONY: all build upload clean connect test-uart test-spi
