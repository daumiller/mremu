#include "registers.hpp"
#include "helpers.hpp"

InterfaceRegisters::InterfaceRegisters(RoscoM68K* rosco, int x, int y) {
  this->rosco = rosco;
  this->x = x;
  this->y = y;

  this->color_background         = 0x444444;
  this->color_border             = 0xFFFFFF;
  this->color_fg_name            = 0x999999;
  this->color_fg_value_unchanged = 0xDDDDDD;
  this->color_fg_value_changed   = 0xDD88FF;

  this->registers = { 0, 0, 0, 0, {0,0,0,0, 0,0,0,0}, {0,0,0,0, 0,0,0,0}, 0 };
}

void InterfaceRegisters::update() {
  // get current registers
  m68k_registers registers_new = this->registers;
  rosco->getRegisters(&registers_new);

  // determine changed registers
  m68k_registers registers_changed = { 0, 0, 0, 0, {0,0,0,0, 0,0,0,0}, {0,0,0,0, 0,0,0,0}, 0 };
  if(registers_new.pc != this->registers.pc) { registers_changed.pc = 1; }
  if(registers_new.sr != this->registers.sr) { registers_changed.sr = 1; }
  for(int idx=0; idx<8; ++idx) {
    if(registers_new.a[idx] != this->registers.a[idx]) { registers_changed.a[idx] = 1; }
    if(registers_new.d[idx] != this->registers.d[idx]) { registers_changed.d[idx] = 1; }
  }
  // determine changed flags
  char c_value = (registers_new.sr & 0x0001) ? 'c' : '-';  uintattr_t c_color = ((registers_new.sr & 0x0001) == (this->registers.sr & 0x0001)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;
  char v_value = (registers_new.sr & 0x0002) ? 'v' : '-';  uintattr_t v_color = ((registers_new.sr & 0x0002) == (this->registers.sr & 0x0002)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;
  char z_value = (registers_new.sr & 0x0004) ? 'z' : '-';  uintattr_t z_color = ((registers_new.sr & 0x0004) == (this->registers.sr & 0x0004)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;
  char n_value = (registers_new.sr & 0x0008) ? 'n' : '-';  uintattr_t n_color = ((registers_new.sr & 0x0008) == (this->registers.sr & 0x0008)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;
  char x_value = (registers_new.sr & 0x0010) ? 'x' : '-';  uintattr_t x_color = ((registers_new.sr & 0x0010) == (this->registers.sr & 0x0010)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;
  char s_value = (registers_new.sr & 0x0100) ? 's' : '-';  uintattr_t s_color = ((registers_new.sr & 0x0100) == (this->registers.sr & 0x0100)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;
  char t_value = (registers_new.sr & 0x8000) ? 't' : '-';  uintattr_t t_color = ((registers_new.sr & 0x8000) == (this->registers.sr & 0x8000)) ? this->color_fg_value_unchanged : this->color_fg_value_changed;

  // update saved registers
  this->registers = registers_new;

  int width  = 2 /*borders*/ + 2 /*x-padding*/ + 6 /* (2 register name + 1 padding) *2 */ + 16 /* 32bit hex *2 */ + 2 /*padding between colums*/;
  int height = 2 /*border*/ + 9 /*rows*/ + 1 /*bit label row*/ + 1 /*spacing row*/;
  helper_draw_box(this->x, this->y, width, height, this->color_border, this->color_border, this->color_background, true);

  uintattr_t data_color;
  // draw Ds & As
  for(int idx=0; idx<8; ++idx) {
    data_color = registers_changed.d[idx] ? this->color_fg_value_changed : this->color_fg_value_unchanged;
    tb_printf(this->x + 2, this->y + 1 + idx, this->color_fg_name, this->color_background, "D%d", idx);
    tb_printf(this->x + 5, this->y + 1 + idx, data_color,          this->color_background, "%08x", registers_new.d[idx]);

    data_color = registers_changed.a[idx] ? this->color_fg_value_changed : this->color_fg_value_unchanged;
    tb_printf(this->x + 15, this->y + 1 + idx, this->color_fg_name, this->color_background, "A%d", idx);
    tb_printf(this->x + 18, this->y + 1 + idx, data_color,          this->color_background, "%08x", registers_new.a[idx]);
  }

  // draw PC
  data_color = registers_changed.pc ? this->color_fg_value_changed : this->color_fg_value_unchanged;
  tb_print( this->x + 2, this->y + 10, this->color_fg_name, this->color_background, "PC");
  tb_printf(this->x + 5, this->y + 10, data_color,          this->color_background, "%08x", registers_new.pc);

  // draw SR
  tb_print( this->x + 15, this->y + 10, this->color_fg_name, this->color_background, "SR");
  tb_printf(this->x + 18, this->y + 10, t_color,             this->color_background, "%c", t_value);
  tb_printf(this->x + 19, this->y + 10, s_color,             this->color_background, "%c", s_value);
  tb_printf(this->x + 20, this->y + 10, x_color,             this->color_background, "%c", x_value);
  tb_printf(this->x + 21, this->y + 10, n_color,             this->color_background, "%c", n_value);
  tb_printf(this->x + 22, this->y + 10, z_color,             this->color_background, "%c", z_value);
  tb_printf(this->x + 23, this->y + 10, v_color,             this->color_background, "%c", v_value);
  tb_printf(this->x + 24, this->y + 10, c_color,             this->color_background, "%c", c_value);
  tb_print( this->x + 18, this->y + 11, this->color_fg_name, this->color_background, "tsxnzvc");
}
