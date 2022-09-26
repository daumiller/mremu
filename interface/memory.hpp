#pragma once

#include <stdint.h>
#include "../machine/rosco_m68k.hpp"

extern "C" {
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

class InterfaceMemory {
public:
  InterfaceMemory(RoscoM68K* rosco, int x, int y, int height);
  ~InterfaceMemory();

  uintattr_t color_background;
  uintattr_t color_border;
  uintattr_t color_fg_address;
  uintattr_t color_fg_data;

  void update();
  bool handleEvent(struct tb_event* event);

  uint32_t getAddress();
  uint32_t setAddress(uint32_t address);

private:
  void updateAddressInput();
  RoscoM68K* rosco;
  int x, y, height, width;
  uint32_t address;
  bool     address_input_editing;
  uint32_t address_input_value;
  uint8_t  address_input_index;
};
