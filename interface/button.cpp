#include <string.h>
#include "button.hpp"
#include "helpers.hpp"

static uint32_t underlinable_label_length(const char* label) {
  if(!label || !label[0]) { return 0; }

  uint32_t length = 0;
  while(*label) {
    if(label[0] == '&') {
      if(label[1] != ' ') {
        ++label;
        continue;
      }
    }

    ++label;
    ++length;
  }

  return length;
}

InterfaceButton::InterfaceButton(int x, int y, int width, int height, const char* label, interface_button_click_handler on_click, void* on_click_data) {
  this->x = x;
  this->y = y;
  this->width = width;
  this->height = height;
  this->label = strdup(label);
  this->on_click = on_click;
  this->on_click_data = on_click_data;

  uint32_t label_length = underlinable_label_length(this->label);
  if(this->width < (label_length+2)) { this->width = label_length + 2; }
  if(this->height < 3) { this->height = 3; } else { this->height |= 1; /* must be odd */ }

  this->color_border_light = 0xBBBBBB;
  this->color_border_dark  = 0x222222;
  this->color_background   = 0x444477;
  this->color_label        = 0xDDDDDD;
}

InterfaceButton::~InterfaceButton() {
  if(this->label) { free(this->label); }
}

bool InterfaceButton::handleEvent(struct tb_event* event) {
  if(event->type != TB_EVENT_MOUSE)        { return false; }
  if(event->x < this->x)                   { return false; }
  if(event->y < this->y)                   { return false; }
  if(event->x >= (this->x + this->width )) { return false; }
  if(event->y >= (this->y + this->height)) { return false; }
  if(event->key != TB_KEY_MOUSE_LEFT)      { return false; }
  if(this->on_click) { this->on_click(this->on_click_data); }
  return true;
}

void InterfaceButton::update() {
  helper_draw_box(this->x, this->y, this->width, this->height, this->color_border_light, this->color_border_dark, this->color_background, true);

  int label_length = underlinable_label_length(this->label);
  int label_x = this->x + 1 + (((this->width - 2) - label_length) >> 1);
  int label_y = this->y + (this->height >> 1);

  char* label_tmp = this->label;
  while(*label_tmp) {
    if((label_tmp[0] == '&') && (label_tmp[1] != ' ')) {
      ++label_tmp;
      tb_printf(label_x++, label_y, this->color_label | TB_TRUECOLOR_UNDERLINE, this->color_background, "%c", *label_tmp);
    } else {
      tb_printf(label_x++, label_y, this->color_label, this->color_background, "%c", *label_tmp);
    }
    ++label_tmp;
  }
}
