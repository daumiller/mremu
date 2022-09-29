#pragma once

extern "C" {
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
#include <vterm/vterm.h>
#include <stdint.h>
}

typedef void (*terminalEventForwarder)(uint32_t event_data, void* callback_data);

class InterfaceTerminal {
public:
  InterfaceTerminal(int x, int y, int width, int height);
  ~InterfaceTerminal();

  void input(uint8_t* data, uint32_t length);
  void update();
  bool handleEvent(struct tb_event* event);
  void setEventForwarder(terminalEventForwarder forwarder, void* forwarder_callback_data);

  uintattr_t color_background;
  uintattr_t color_border;

private:
  int x, y, width, height;
  bool input_captured;
  VTerm* vterm;
  VTermScreen* vterm_screen;
  terminalEventForwarder event_forwarder;
  void* event_forwarder_data;
};
