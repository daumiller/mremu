#pragma once
#include <functional>

extern "C" {
#include <stdbool.h>
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

typedef void (*interface_button_click_handler)(void*);

class InterfaceButton {
public:
  InterfaceButton(int x, int y, int width, int height, const char* label, interface_button_click_handler on_click, void* on_click_data);
  ~InterfaceButton();
  bool handleEvent(struct tb_event* event);
  void update();

  uintattr_t color_border_light;
  uintattr_t color_border_dark;
  uintattr_t color_background;
  uintattr_t color_label;

  interface_button_click_handler on_click;
  void* on_click_data;

private:
  char* label;
  int x, y, width, height;
};
