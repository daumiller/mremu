#pragma once

extern "C" {
#include <stdbool.h>
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

void helper_draw_box(int x, int y, int width, int height, uintattr_t color_border_light, uintattr_t color_border_dark, uintattr_t color_background, bool rounded);
