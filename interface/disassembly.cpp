#include "disassembly.hpp"
#include "helpers.hpp"

InterfaceDisassembly::InterfaceDisassembly(RoscoM68K* rosco, uint8_t ins_count_previous, uint8_t ins_count_future, int x, int y, int width) {
  this->rosco = rosco;
  this->x = x;
  this->y = y;

  if(width <   8) { width =   8; }
  if(width > 128) { width = 128; }
  this->width = width;

  this->color_background  = 0x444444;
  this->color_border      = 0xFFFFFF;
  this->color_fg_previous = 0xDDDDDD;
  this->color_fg_current  = 0xDD88FF;
  this->color_fg_future   = 0x999999;

  this->line_previous_count = ins_count_previous;
  this->line_previous = (char**)malloc(sizeof(char*) * ins_count_previous);
  for(uint8_t idx=0; idx<ins_count_previous; ++idx) {
    this->line_previous[idx] = (char*)malloc(64);
    this->line_previous[idx][0] = 0x00;
  }

  this->line_future_count = ins_count_future;
}

InterfaceDisassembly::~InterfaceDisassembly() {
  if(this->line_previous) {
    for(uint8_t idx=0; idx<this->line_previous_count; ++idx) { free(this->line_previous[idx]); }
    free(this->line_previous);
  }
}

void InterfaceDisassembly::update() {
  static char disassembly_buffer[128];
  static char printing_buffer[128];

  int width  = this->width;
  int height = this->line_previous_count + this->line_future_count + 1 + 2;
  helper_draw_box(this->x, this->y, this->width, height, this->color_border, this->color_border, this->color_background, true);

  int max_string_width = this->width - 4;
  // draw previous lines
  int y_offset = this->y + 1;
  for(int idx=this->line_previous_count-1; idx>=0; --idx) {
    if(this->line_previous[idx][0]) {
      tb_printf(this->x + 2, y_offset++, this->color_fg_previous, this->color_background, "%s", this->line_previous[idx]);
    }
  }

  // get/draw current line
  uint32_t pc_tmp = rosco->getPC();
  uint32_t pc_next = pc_tmp + rosco->disassemble(pc_tmp, disassembly_buffer, moira::DASM_MOIRA_MOT);
  snprintf(printing_buffer, max_string_width, "%06x > %s", pc_tmp, disassembly_buffer);
  tb_print(this->x + 2, y_offset++, this->color_fg_current, this->color_background, printing_buffer);

  // move back previous lines
  if(this->line_previous_count) {
    for(int idx=this->line_previous_count-1; idx>0; --idx) {
      sprintf(this->line_previous[idx], "%s", this->line_previous[idx-1]);
    }
    snprintf(this->line_previous[0], max_string_width, "%06x   %s", pc_tmp, disassembly_buffer);
  }

  // draw future lines
  for(uint8_t idx=0; idx<this->line_future_count; ++idx) {
    pc_tmp = pc_next;
    pc_next += rosco->disassemble(pc_next, disassembly_buffer, moira::DASM_MOIRA_MOT);
    snprintf(printing_buffer, max_string_width, "%06x   %s", pc_tmp, disassembly_buffer);
    tb_print(this->x + 2, y_offset++, this->color_fg_future, this->color_background, printing_buffer);
  }
}
