#include "memory.hpp"
#include "helpers.hpp"

static bool memory_read(RoscoM68K* rosco, uint32_t address, uint8_t* byte, char* ascii) {
  uint32_t ram_lower, ram_upper;
  uint32_t rom_lower, rom_upper;
  rosco->addressExtentsRam(&ram_lower, &ram_upper);
  rosco->addressExtentsRom(&rom_lower, &rom_upper);
  bool addressInRam = (address >= ram_lower) && (address <= ram_upper);
  bool addressInRom = (address >= rom_lower) && (address <= rom_upper);
  if(!addressInRam && !addressInRom) {
    if(byte ) { *byte  = 0x00; }
    if(ascii) { *ascii = '?';  }
    return false;
  }

  uint8_t data = rosco->busRead(address);
  if(byte) { *byte = data; }
  if(ascii) {
    if(data < 0x20) {
      *ascii = '.';
    } else if(data < 0x7F) {
      *ascii = (char)data;
    } else {
      *ascii = '.';
    }
  }
  return true;
}

InterfaceMemory::InterfaceMemory(RoscoM68K* rosco, int x, int y,int height) {
  this->rosco = rosco;
  this->x = x;
  this->y = y;
  this->height = (height > 4) ? height : 4;
  this->width = 2 /*border*/ + 2 /*margin*/ + 8 /*address+space*/ + 32 /*sixteen bytes*/ + 15 /*spaces after 15 bytes*/ + 18 /*space_dump*/;
  this->address = 0x000000;
  this->color_background = 0x444444;
  this->color_border     = 0xFFFFFF;
  this->color_fg_address = 0x999999;
  this->color_fg_data    = 0xDDDDDD;
  this->address_input_editing = false;
  this->address_input_value   = 0x000000;
  this->address_input_index   = 0;
}

InterfaceMemory::~InterfaceMemory() {
}

bool InterfaceMemory::handleEvent(struct tb_event* event) {
  int input_y     = this->y + 1;
  int input_x_min = this->x + 2;
  int input_x_max = input_x_min + 5;

  // click inside address box; start editing
  if(!this->address_input_editing) {
    if(event->type != TB_EVENT_MOUSE)   { return false; }
    if(event->y != input_y   )          { return false; }
    if(event->x < input_x_min)          { return false; }
    if(event->x > input_x_max)          { return false; }
    if(event->key != TB_KEY_MOUSE_LEFT) { return false; }

    this->address_input_editing = true;
    this->address_input_index   = 0;
    this->address_input_value   = this->address;
    this->updateAddressInput();
    return true;
  }

  // click outside of address box; stop editing
  if(event->type == TB_EVENT_MOUSE) {
    if((event->y != input_y) || (event->x < input_x_min) || (event->x > input_x_max)) {
      this->address_input_editing = false;
      this->updateAddressInput();
      return false;
    }
  }

  if(event->type == TB_EVENT_KEY) {
    // pressed escape; stop editing
    if(event->key == TB_KEY_ESC) {
      this->address_input_editing = false;
      this->updateAddressInput();
      return true;
    }

    // pressed non-hex digit; ignore
    if(event->ch < '0') { return true; }
    if(event->ch > 'f') { return true; }
    if((event->ch > '9') && (event->ch < 'a')) { return true; }

    // pressed hex digit; proceed with editing
    uint32_t hex_digit = (event->ch <= '9') ? (event->ch - '0') : (event->ch - 'a' + 10);
    uint32_t hex_shift = (20 - (this->address_input_index * 4));
    hex_digit <<= hex_shift;
    this->address_input_value &= ~(0xF << hex_shift);
    this->address_input_value |= hex_digit;
    this->address_input_index += 1;
    if(this->address_input_index == 6) {
      this->setAddress(this->address_input_value);
      this->address_input_editing = false;
      this->update();
    } else {
      this->updateAddressInput();
    }
    return true;
  }

  return false;
}

uint32_t InterfaceMemory::getAddress() {
  return this->address;
}

uint32_t InterfaceMemory::setAddress(uint32_t address) {
  if(address > 0xFFFFFF) { address = 0xFFFFFF; }
  this->address = address & 0xFFFFF0; // align to 16 byte boundary
  return this->address;
}

void InterfaceMemory::updateAddressInput() {
  if(!this->address_input_editing) {
    tb_printf(this->x + 2, this->y + 1, 0xDD88FF, 0x000000, "%06x", this->address);
    return;
  }

  uint32_t mask = 0xF00000;
  uint32_t mask_shift = 20;
  int x_offset = this->x + 2;
  for(int hex_digit=0; hex_digit<6; ++hex_digit) {
    if(hex_digit < this->address_input_index) {
      uint32_t current_digit = (this->address_input_value & mask) >> mask_shift;
      tb_printf(x_offset, this->y + 1, 0x884488, 0xBBBBFF, "%x", current_digit);
    } else if(hex_digit == this->address_input_index) {
      tb_print(x_offset, this->y + 1, 0x000000, 0x0000FF, " ");
    } else {
      uint32_t current_digit = (this->address & mask) >> mask_shift;
      tb_printf(x_offset, this->y + 1, 0x333333, 0xBBBBFF, "%x", current_digit);
    }

    mask >>= 4;
    mask_shift -= 4;
    ++x_offset;
  }
}

void InterfaceMemory::update() {
  helper_draw_box(this->x, this->y, this->width, this->height, this->color_border, this->color_border, this->color_background, true);

  this->updateAddressInput();

  tb_print(this->x + 10, this->y + 1, this->color_fg_address, this->color_background, " 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  0123456789abcdef");
  int memory_lines = height-3;

  uint32_t address = this->address;
  uint8_t data_byte;
  char data_ascii;
  int y = this->y + 2;
  for(int idx=0; idx<memory_lines; ++idx) {
    int x = this->x + 2;
    if(address < 0x1000000) {
      tb_printf(x, y, this->color_fg_address, this->color_background, "%06x", address);
      tb_printf(x+5, y, this->color_fg_address, this->color_background, "-"); // overwrite last digit of address
    } else {
      tb_printf(x, y, this->color_fg_address, this->color_background, "      ", address);
    }
    x += 8;
    
    for(int byte_idx=0; byte_idx<16; ++byte_idx) {
      bool success = memory_read(this->rosco, address + byte_idx, &data_byte, &data_ascii);
      if(success) {
        tb_printf(x + (byte_idx*3), y, this->color_fg_data, this->color_background, "%02x ", data_byte);
        tb_printf(x + 49 + byte_idx, y, this->color_fg_data, this->color_background, "%c", data_ascii);
      } else {
        tb_print(x + (byte_idx*3), y, this->color_fg_data, this->color_background, "-- ");
        tb_print(x + 49 + byte_idx, y, this->color_fg_data, this->color_background, " ");
      }
    }

    ++y;
    address += 16;
  }
}
