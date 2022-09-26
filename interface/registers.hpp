#pragma once

#include <stdint.h>
#include "../machine/rosco_m68k.hpp"

extern "C" {
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

class InterfaceRegisters {
public:
  InterfaceRegisters(RoscoM68K* rosco, int x, int y);

  void update();
  uintattr_t color_background;
  uintattr_t color_border;
  uintattr_t color_fg_name;
  uintattr_t color_fg_value_unchanged;
  uintattr_t color_fg_value_changed;

private:
  RoscoM68K* rosco;
  int x, y;
  m68k_registers registers;
};
