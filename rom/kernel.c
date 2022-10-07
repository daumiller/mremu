#define NULL ((void*)0x00000000)
#include <stdint.h>
#include <stdbool.h>
#include "duart.h"

extern void runProgramA();
extern void runProgramB();

void whatShouldWeDo() {
  char temp[128];

  duartWriteString("\r\n? > ");
  duartReadLine(temp, 127, NULL, true);

  if((temp[0] == 'p') && (temp[1] == 'a') && (temp[2] == 0x00)) { __asm__ ("jmp 0x00001338:l"); }
  if((temp[0] == 'p') && (temp[1] == 'b') && (temp[2] == 0x00)) { __asm__ ("jmp 0x0000BABE:l"); }

  duartWriteString("Sorry. I don't know how to: \"");
  duartWriteString(temp);
  duartWriteString("\"...\r\n");
}

void startup() {
  duartWriteString("=== c startup entry point ===\r\n");

  while(true) {
    whatShouldWeDo();
  }
}
