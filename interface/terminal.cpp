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
  helper_draw_box(this->x, this->y, this->width, this->height, this->color_border, this->color_border, this->color_background, true);

  int rows = this->height - 2;
  int cols = this->width  - 2;

  VTermScreenCell cell;
  uintattr_t color_fg;
  uintattr_t color_bg;
  for(int row=0; row<rows; ++row) {
    for(int col=0; col<cols; ++col) {
      VTermPos cell_position = { .row = row, .col = col };
      vterm_screen_get_cell(this->vterm_screen, cell_position, &cell);
      color_fg = (((uint32_t)(cell.fg.rgb.red  )) << 16) |
                 (((uint32_t)(cell.fg.rgb.green)) <<  8) |
                 (((uint32_t)(cell.fg.rgb.blue )) <<  0) ;
      color_bg = (((uint32_t)(cell.bg.rgb.red  )) << 16) |
                 (((uint32_t)(cell.bg.rgb.green)) <<  8) |
                 (((uint32_t)(cell.bg.rgb.blue )) <<  0) ;
      if(cell.attrs.bold     ) { color_fg |= TB_TRUECOLOR_BOLD;      }
      if(cell.attrs.underline) { color_fg |= TB_TRUECOLOR_UNDERLINE; }
      if(cell.attrs.italic   ) { color_fg |= TB_TRUECOLOR_ITALIC;    }
      if(cell.attrs.blink    ) { color_fg |= TB_TRUECOLOR_BLINK;     }
      if(cell.attrs.reverse  ) { color_fg |= TB_TRUECOLOR_REVERSE;   } 
      tb_printf(this->x + 1 + col, this->y + 1 + row, color_fg, color_bg, "%c", cell.chars[0] ? cell.chars[0] : ' ');
    }
  }
}

bool InterfaceTerminal::handleEvent(struct tb_event* event) {
  return false;
}
