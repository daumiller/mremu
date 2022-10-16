#define NULL ((void*)0x00000000)
#include <stdint.h>
#include <stdbool.h>
#include "duart.h"

void startup() {
  char temp[128];
  duartWriteString("Hello world of C on 68k!\r\n");

  duartWriteString("What should we do today? > ");
  duartReadLine(temp, 127, NULL, true);
  duartWriteString("\r\n");

  duartWriteString("Sorry. I don't know how to: ");
  duartWriteLine(temp);
}
