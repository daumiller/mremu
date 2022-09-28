#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// clang -std=c99 -L./ -lvterm test.c -o test
#include "vterm.h"

typedef struct {
  VTerm*       vterm;
  VTermScreen* vterm_screen;
  int          row_count;
  int          column_count;
} AppState;

int main(int argc, char** argv) {
  AppState state = { NULL, NULL, 20, 70 };
  state.vterm = vterm_new(state.row_count, state.column_count);
  vterm_set_utf8(state.vterm, 0);

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

  state.vterm_screen = vterm_obtain_screen(state.vterm);
  vterm_screen_set_callbacks(state.vterm_screen, &vterm_screen_callbacks, &state);
  vterm_screen_reset(state.vterm_screen, 1);

  FILE* fin = fopen("test.text", "rb");
  char buffer[16];
  size_t read_length = fread(buffer, 1, 16, fin);
  while(read_length) {
    vterm_input_write(state.vterm, buffer, read_length);
    read_length = fread(buffer, 1, 16, fin);
  }
  fclose(fin);

  VTermScreenCell cell;
  for(int row=0; row<state.row_count; ++row) {
    printf("%02d: ", row);
    for(int col=0; col<state.column_count; ++col) {
      VTermPos cell_position = { .row = row, .col = col };
      vterm_screen_get_cell(state.vterm_screen, cell_position, &cell);
      printf("%c", cell.chars[0]);
    }
    printf("\n");
  }

  vterm_free(state.vterm);
  return 0;
}
