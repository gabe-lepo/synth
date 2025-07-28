#include "twi_util.h"
#include "../../util/debug.h"
#include "../twi.h"
#include <stdio.h>

uint8_t twi_scan(void) {
  uint8_t dev_addr = 0;
  printf("Scanning for i2c devices...\n");

  for (uint8_t addr = 1; addr <= 127; addr++) {
    twi_start();
    if (!twi_addressWrite(addr)) {
      printf("Found device at: 0x%02X\n", addr);
      dev_addr = addr;
    }
    twi_stop();
  }

  return dev_addr;
}
