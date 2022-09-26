#pragma once

#include <stdint.h>
#include "../machine/rosco_m68k.hpp"

extern "C" {
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

class InterfaceDisassembly {
public:
  InterfaceDisassembly(RoscoM68K* rosco, uint8_t ins_count_previous, uint8_t ins_count_future, int x, int y, int width);
  ~InterfaceDisassembly();

  void update();
  uintattr_t color_background;
  uintattr_t color_border;
  uintattr_t color_fg_previous;
  uintattr_t color_fg_current;
  uintattr_t color_fg_future;

private:
  RoscoM68K* rosco;
  int x, y;
  int width;
  uint8_t line_previous_count;
  uint8_t line_future_count;
  char** line_previous;
};
