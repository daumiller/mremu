#include <stdint.h>
#include <stdbool.h>
#include "duart.h"
#include "sdcard.h"

static void hexString24(uint32_t hex, uint8_t* buffer) {
  for(uint8_t idx=0; idx<6; ++idx) {
    uint8_t nibble = (hex & 0xF00000) >> 20;
    if(nibble < 10) {
      buffer[0] = '0' + nibble;
    } else {
      buffer[0] = 'A' + nibble - 10;
    }
    ++buffer;
    hex <<= 4;
  }
}

void main(uint32_t startup_location) {
  duartOutput_enable();

  uint8_t hex24[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  hexString24(startup_location, hex24);
  duartSerial_writeString(DUART_SERIAL_A, "Relocatable code running from 0x");
  duartSerial_writeString(DUART_SERIAL_A, hex24);
  duartSerial_writeString(DUART_SERIAL_A, "\r\n");

  duartSerial_writeString(DUART_SERIAL_A, "Looking for SD card... ");
  SdCard sdcard;
  bool has_sdcard = sdCard_initialize(&sdcard, DUART_SPI_B);
  if(has_sdcard) {
    duartSerial_writeString(DUART_SERIAL_A, "Found!\r\n");
  } else {
    duartSerial_writeString(DUART_SERIAL_A, "not found\r\n");
  }

  bool on_off = true;
  while(true) {
    if(on_off) {
      duartLed_set(DUART_LED_GREEN, true);
      duartLed_set(DUART_LED_RED, false);
    } else {
      duartLed_set(DUART_LED_GREEN, false);
      duartLed_set(DUART_LED_RED, true);
    }
    for(uint16_t idx=0; idx<65534; ++idx) {}
    on_off = !on_off;
  }
}
