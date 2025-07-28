#!/bin/zsh

# AVR Memory Usage Checker for ATmega328P

# ATmega328P mem limits
FLASH_SIZE=32768
SRAM_SIZE=2048

# Fancy output colors
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

ELF_PATH="build/main.elf"

# Check if file exists
if [ ! -f ${ELF_PATH} ]; then
  echo -e "${RED}Error: build/main.elf not found!${NC}"
  exit 1
fi

echo -e "${BLUE}=== ATmega328P Memory Usage Report ===${NC}"
echo

# Run avr-size and capture output
SIZE_OUTPUT=$(avr-size build/main.elf)

# Extract the numbers (skip header line)
NUMBERS=$(echo "$SIZE_OUTPUT" | tail -n 1)

# Parse the values
TEXT=$(echo $NUMBERS | awk '{print $1}')
DATA=$(echo $NUMBERS | awk '{print $2}')
BSS=$(echo $NUMBERS | awk '{print $3}')

# Calculate memory usage
FLASH_USED=$((TEXT + DATA))
SRAM_USED=$((DATA + BSS))

# Calculate percentages
FLASH_PERCENT=$(echo "scale=1; $FLASH_USED * 100 / $FLASH_SIZE" | bc -l)
SRAM_PERCENT=$(echo "scale=1; $SRAM_USED * 100 / $SRAM_SIZE" | bc -l)

# Calculate remaining
FLASH_REMAINING=$((FLASH_SIZE - FLASH_USED))
SRAM_REMAINING=$((SRAM_SIZE - SRAM_USED))

# Display raw avr-size output
echo -e "${BLUE}Raw avr-size output:${NC}"
echo "$SIZE_OUTPUT"
echo

# Display detailed breakdown
echo -e "${BLUE}Detailed Memory Breakdown:${NC}"
echo -e "  Program Code (text): ${TEXT} bytes"
echo -e "  Initialized Data:    ${DATA} bytes"
echo -e "  Uninitialized Data:  ${BSS} bytes"
echo

# Flash usage with color coding
echo -e "${BLUE}Flash Memory Usage:${NC}"
if (($(echo "$FLASH_PERCENT < 75" | bc -l))); then
  COLOR=$GREEN
elif (($(echo "$FLASH_PERCENT < 90" | bc -l))); then
  COLOR=$YELLOW
else
  COLOR=$RED
fi
echo -e "  Used: ${COLOR}${FLASH_USED}${NC} / ${FLASH_SIZE} bytes (${COLOR}${FLASH_PERCENT}%${NC})"
echo -e "  Free: ${FLASH_REMAINING} bytes"
echo

# SRAM usage with color coding
echo -e "${BLUE}SRAM Usage:${NC}"
if (($(echo "$SRAM_PERCENT < 60" | bc -l))); then
  COLOR=$GREEN
elif (($(echo "$SRAM_PERCENT < 80" | bc -l))); then
  COLOR=$YELLOW
else
  COLOR=$RED
fi
echo -e "  Used: ${COLOR}${SRAM_USED}${NC} / ${SRAM_SIZE} bytes (${COLOR}${SRAM_PERCENT}%${NC})"
echo -e "  Free: ${SRAM_REMAINING} bytes"
echo

# Warnings and recommendations
if (($(echo "$SRAM_PERCENT > 75" | bc -l))); then
  echo -e "${RED}⚠️  WARNING: High SRAM usage! Consider optimizing.${NC}"
fi

if (($(echo "$FLASH_PERCENT > 90" | bc -l))); then
  echo -e "${RED}⚠️  WARNING: High Flash usage!${NC}"
fi

if ((DATA > 200)); then
  echo -e "${YELLOW}💡 TIP: Large data section (${DATA} bytes). Consider using PROGMEM for constants.${NC}"
fi

# Stack space estimate
STACK_ESTIMATE=250
AVAILABLE_FOR_VARIABLES=$((SRAM_REMAINING - STACK_ESTIMATE))
echo -e "${BLUE}Estimated available for variables:${NC} ~${AVAILABLE_FOR_VARIABLES} bytes"
echo -e "  (Assuming ~${STACK_ESTIMATE} bytes needed for stack)"
echo
