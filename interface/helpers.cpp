#include "helpers.hpp"

void helper_draw_box(int x, int y, int width, int height, uintattr_t color_border_light, uintattr_t color_border_dark, uintattr_t color_background, bool rounded) {
  int max_x  = x + width  - 1;
  int max_y  = y + height - 1;

  const char* str_top_left     = rounded ? "╭" : "┌";
  const char* str_bottom_left  = rounded ? "╰" : "└";
  const char* str_top_right    = rounded ? "╮" : "┐";
  const char* str_bottom_right = rounded ? "╯" : "┘";

  // draw border
  tb_print(x,     y,     color_border_light, color_background, str_top_left);
  tb_print(x,     max_y, color_border_light, color_background, str_bottom_left);
  tb_print(max_x, y,     color_border_dark,  color_background, str_top_right);
  tb_print(max_x, max_y, color_border_dark,  color_background, str_bottom_right);
  for(int idx=1; idx<width-1; ++idx) {
    tb_print(x + idx, y,     color_border_light, color_background, "─");
    tb_print(x + idx, max_y, color_border_dark,  color_background, "─");
  }
  for(int idx=1; idx<height-1; ++idx) {
    tb_print(x,     y + idx, color_border_light, color_background, "│");
    tb_print(max_x, y + idx, color_border_dark,  color_background, "│");
  }

  // clear non-bordered area
  for(int xidx=1; xidx<width-1; ++xidx) {
    for(int yidx=1; yidx<height-1; ++yidx) {
      tb_print(x+xidx, y+yidx, color_background, color_background, " ");
    }
  }
}
