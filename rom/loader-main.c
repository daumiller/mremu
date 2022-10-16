#define NULL ((void*)0x00000000)
#include <stdint.h>
#include <stdbool.h>
#include "duart.h"

void loader_main() {
  duartWriteString("\r\n");

  while(true) {
    duartWriteString("=== waiting for program ===\r\n");
    uint32_t programSize = duartReadLong();

    if(programSize == 0) {
      duartWriteString("  Received invalid program size of 0.\r\n");
      continue;
    }
    if(programSize >= 1024000) {
      duartWriteString("  Program size too large (> 1024 KB).\r\n");
      continue;
    }

    uint8_t* program_area = (uint8_t*)(0x00000410);
    duartReadBytes(program_area, programSize);
    duartWriteString("  Program loaded. Starting...\r\n");

    __asm__ volatile ("  jmp (%0) \r\n"
                  :
                  : "a" (program_area));
  }
}
