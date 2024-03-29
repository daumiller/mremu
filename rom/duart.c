#include <stdint.h>
#include <stdbool.h>
#include "duart.h"

#define DUART_REG_STATUS ((volatile char*)0xF00003) /* DUART_REG_SRA */
#define DUART_REG_TXRX   ((volatile char*)0xF00007) /* read:DUART_REG_RHRA, write:DUART_REG_THRA */

void duartWriteByte(uint8_t byte) {
  while((*DUART_REG_STATUS & 0x4) == 0) { ; } // wait until tx ready
  *DUART_REG_TXRX = byte;
}

void duartWriteString(const char* string) {
  while(*string) {
    while((*DUART_REG_STATUS & 0x4) == 0) { ; } // wait until tx ready
    *DUART_REG_TXRX = *string;
    ++string;
  }
}

void duartWriteLine(const char* string) {
  duartWriteString(string);
  duartWriteByte(0x0D);
  duartWriteByte(0x0A);
}

uint8_t duartReadByte(bool echo) {
  while((*DUART_REG_STATUS & 0x1) == 0) { ; } // wait until rx ready
  uint8_t read = *DUART_REG_TXRX;
  if(echo) { duartWriteByte(read); }
  return read;
}

void duartReadLine(char* buffer, uint16_t max_length, uint16_t* read_length, bool echo) {
  uint16_t read_count = 0;
  while(read_count < max_length) {
    *buffer = (char)duartReadByte(echo);
    if(*buffer == 0x0D) { continue; }
    if(*buffer == 0x0A) { break; }
    ++buffer;
  }
  *buffer = 0x00;
  if(read_length) {
    *read_length = read_count;
  }
}

uint32_t duartReadLong() {
  uint32_t result;

  __asm__ volatile ("  move.l %0,%%a0   \r\n"
                    "  move.l #0x4,%%d0 \r\n"
                    "  trap #0x1      \r\n"
                    :
                    : "a" (&result)
                    : "%%a0", "%%d0", "%%d1");

  return result;
}

void duartReadBytes(uint8_t* buffer, uint32_t count) {
  __asm__ volatile ("  move.l %0,%%a0 \r\n"
                    "  move.l %1,%%d0 \r\n"
                    "  trap #0x1    \r\n"
                    :
                    : "a" (buffer), "d" (count)
                    : "%%a0", "%%d0", "%%d1");
}
