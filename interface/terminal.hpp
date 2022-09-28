#pragma once

extern "C" {
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
#include <vterm/vterm.h>
#include <stdint.h>
}

class InterfaceTerminal {
public:
  InterfaceTerminal(int x, int y, int width, int height);
  ~InterfaceTerminal();

  void input(uint8_t* data, uint32_t length);
  void update();
  bool handleEvent(struct tb_event* event);
  uintattr_t color_background;
  uintattr_t color_border;

private:
  int x, y, width, height;
  VTerm* vterm;
  VTermScreen* vterm_screen;
};
