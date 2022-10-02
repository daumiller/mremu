#include <stdint.h>

#define DUART_REG_TXRX ((volatile char*)0xF00007) /* read:DUART_REG_RHRA, write:DUART_REG_THRA */

static void printHex24(uint32_t hex) {
  for(uint8_t idx=0; idx<6; ++idx) {
    uint8_t nibble = (hex & 0xF00000) >> 20;
    if(nibble < 10) {
      *DUART_REG_TXRX = '0' + nibble;
    } else {
      nibble -= 10;
      *DUART_REG_TXRX = 'A' + nibble;
    }
    hex <<= 4;
  }
}

static void printString(const char* string) {
  while(*string) {
    *DUART_REG_TXRX = *string;
    ++string;
  }
}

void main(uint32_t startup_location) {
  printString("Relocatable code running from 0x");
  printHex24(startup_location);
  printString("\r\n");
}
