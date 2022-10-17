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

static void decString(uint32_t dec, uint8_t* buffer) {
  uint8_t dec_buffer[16];
  uint8_t digit_number = 0;
  uint8_t dec_buffer_location = 15;
  while(true) {
    uint32_t digit = dec % 10;
    dec -= digit;
    dec /= 10;
    dec_buffer[dec_buffer_location] = (uint8_t)(digit + '0');
    if(dec == 0) { break; }
    dec_buffer_location--;
    digit_number++;
    if(digit_number %3 == 0) {
      dec_buffer[dec_buffer_location] = ',';
      dec_buffer_location--;
    }
  }

  for(uint8_t idx=dec_buffer_location; idx<16; ++idx) {
    buffer[0] = dec_buffer[idx];
    buffer++;
  }
  buffer[0] = 0x00;
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
    duartSerial_writeString(DUART_SERIAL_A, "  SD Card Capacity: ");
    uint8_t decimal_buffer[16];
    decString(sdcard.size_in_mib, decimal_buffer);
    duartSerial_writeString(DUART_SERIAL_A, decimal_buffer);
    duartSerial_writeString(DUART_SERIAL_A, " MiB\r\n");
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
