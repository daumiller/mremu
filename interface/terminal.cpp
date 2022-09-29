#include "terminal.hpp"
#include "helpers.hpp"

InterfaceTerminal::InterfaceTerminal(int x, int y, int width, int height) {
  if(width  < 8) { width  = 8; }
  if(height < 3) { height = 3; }

  this->x = x;
  this->y = y;
  this->width  = width;
  this->height = height;
  this->color_background = 0x444444;
  this->color_border     = 0xFFFFFF;
  this->input_captured   = false;
  this->event_forwarder      = NULL;
  this->event_forwarder_data = NULL;

  this->vterm = vterm_new(this->height-2, this->width-2);
  vterm_set_utf8(this->vterm, 0);
  VTermScreenCallbacks vterm_screen_callbacks = {
    /*
    int (*damage)(VTermRect rect, void *user);
    int (*moverect)(VTermRect dest, VTermRect src, void *user);
    int (*movecursor)(VTermPos pos, VTermPos oldpos, int visible, void *user);
    int (*settermprop)(VTermProp prop, VTermValue *val, void *user);
    int (*bell)(void *user);
    int (*resize)(int rows, int cols, void *user);
    int (*sb_pushline)(int cols, const VTermScreenCell *cells, void *user);
    int (*sb_popline)(int cols, VTermScreenCell *cells, void *user); // try to pop out a pushed row (from the top?); only used for resizing/scrolling
    int (*sb_clear)(void* user);
    */
  };

  this->vterm_screen = vterm_obtain_screen(this->vterm);
  vterm_screen_set_callbacks(this->vterm_screen, NULL /* &vterm_screen_callbacks */, (void*)this);
  vterm_screen_reset(this->vterm_screen, 1);
}

InterfaceTerminal::~InterfaceTerminal() {
  vterm_free(this->vterm);
}

void InterfaceTerminal::input(uint8_t* data, uint32_t length) {
  vterm_input_write(this->vterm, (const char*)data, (size_t)length);
  this->update();
}

void InterfaceTerminal::update() {
  uintattr_t color_border_foreground = this->input_captured ? 0x884488 : this->color_border;
  uintattr_t color_border_background = this->input_captured ? 0xBBBBFF : this->color_background;
  helper_draw_box(this->x, this->y, this->width, this->height, color_border_foreground, color_border_foreground, color_border_background, true);

  int rows = this->height - 2;
  int cols = this->width  - 2;

  VTermScreenCell cell;
  uintattr_t color_fg;
  uintattr_t color_bg;
  VTermColor vt_color_fg;
  VTermColor vt_color_bg;
  for(int row=0; row<rows; ++row) {
    for(int col=0; col<cols; ++col) {
      VTermPos cell_position = { .row = row, .col = col };
      vterm_screen_get_cell(this->vterm_screen, cell_position, &cell);

      vt_color_fg = cell.fg;
      vt_color_bg = cell.bg;
      if(VTERM_COLOR_IS_DEFAULT_FG(&(vt_color_fg))) { vterm_color_rgb(&vt_color_fg, 255, 255, 255); }
      if(VTERM_COLOR_IS_DEFAULT_BG(&(vt_color_bg))) { vterm_color_rgb(&vt_color_bg,   0,   0,   0); }
      if(VTERM_COLOR_IS_INDEXED(&(vt_color_fg))) { vterm_screen_convert_color_to_rgb(this->vterm_screen, &vt_color_fg); }
      if(VTERM_COLOR_IS_INDEXED(&(vt_color_bg))) { vterm_screen_convert_color_to_rgb(this->vterm_screen, &vt_color_bg); }

      // TODO: HACK:
      // not sure why, but bootloader is writing some text (version string) with a black foreground.
      // working around this for now by disallowing a black foreground color
      if((vt_color_fg.rgb.red == 0) && (vt_color_fg.rgb.green == 0) && (vt_color_fg.rgb.blue == 0)) {
        vt_color_fg.rgb.red   = 64;
        vt_color_fg.rgb.green = 64;
        vt_color_fg.rgb.blue  = 64;
      }

      color_fg = (((uint32_t)(vt_color_fg.rgb.red  )) << 16) |
                 (((uint32_t)(vt_color_fg.rgb.green)) <<  8) |
                 (((uint32_t)(vt_color_fg.rgb.blue )) <<  0) ;
      color_bg = (((uint32_t)(vt_color_bg.rgb.red  )) << 16) |
                 (((uint32_t)(vt_color_bg.rgb.green)) <<  8) |
                 (((uint32_t)(vt_color_bg.rgb.blue )) <<  0) ;
      if(cell.attrs.bold     ) { color_fg |= TB_TRUECOLOR_BOLD;      }
      if(cell.attrs.underline) { color_fg |= TB_TRUECOLOR_UNDERLINE; }
      if(cell.attrs.italic   ) { color_fg |= TB_TRUECOLOR_ITALIC;    }
      if(cell.attrs.blink    ) { color_fg |= TB_TRUECOLOR_BLINK;     }
      if(cell.attrs.reverse  ) { color_fg |= TB_TRUECOLOR_REVERSE;   } 
      tb_printf(this->x + 1 + col, this->y + 1 + row, color_fg, color_bg, "%c", cell.chars[0] ? cell.chars[0] : ' ');
    }
  }
}

void InterfaceTerminal::setEventForwarder(terminalEventForwarder forwarder, void* forwarder_callback_data) {
  this->event_forwarder      = forwarder;
  this->event_forwarder_data = forwarder_callback_data;
}

bool InterfaceTerminal::handleEvent(struct tb_event* event) {
  int x_min = this->x;
  int y_min = this->y;
  int x_max = this->x + this->width;
  int y_max = this->y + this->height;

  // click inside terminal; capture input
  if(!this->input_captured) {
    if(event->type != TB_EVENT_MOUSE)   { return false; }
    if(event->y < y_min)                { return false; }
    if(event->y > y_max)                { return false; }
    if(event->x < x_min)                { return false; }
    if(event->x > x_max)                { return false; }
    if(event->key != TB_KEY_MOUSE_LEFT) { return false; }

    this->input_captured = true;
    this->update();
    return true;
  }

  // click outside of terminal; release input
  if(event->type == TB_EVENT_MOUSE) {
    if((event->y < y_min) || (event->y > y_max) || (event->x < x_min) || (event->x > x_max)) {
      this->input_captured = false;
      this->update();
      return false;
    }
  }

  if(event->type == TB_EVENT_KEY) {
    // pressed escape; release input
    if(event->key == TB_KEY_ESC) {
      this->input_captured = false;
      this->update();
      return true;
    }

    // pressed something else; forward it on, if able to
    if(this->event_forwarder) {
      if(event->ch) {
        this->event_forwarder(event->ch, this->event_forwarder_data);
        return true;
      }
      if(event->key == TB_KEY_ENTER) {
        this->event_forwarder(0x0D, this->event_forwarder_data);
        this->event_forwarder(0x0A, this->event_forwarder_data);
        return true;
      }
    }
  }

  return false;
}
