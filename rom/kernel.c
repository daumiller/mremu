#define NULL ((void*)0x00000000)
#include <stdint.h>
#include <stdbool.h>
#include "duart.h"

void whatShouldWeDo() {
  char temp[128];

  duartWriteString("\r\nWhat should we do today? > ");
  duartReadLine(temp, 127, NULL, true);

  duartWriteString("Sorry. I don't know how to: \"");
  duartWriteString(temp);
  duartWriteString("\"...\r\n");
}

void startup() {
  duartWriteString("=== Hello world of C on 68k! ===\r\n");

  while(true) {
    whatShouldWeDo();
  }
}
