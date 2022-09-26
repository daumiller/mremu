#include "rosco_m68k.hpp"

RoscoM68K::RoscoM68K(const char* rom_path) : moira::Moira() {
  FILE* rom_file = fopen(rom_path, "rb");
  if(!rom_file) { throw "error opening rosco rom file"; }

  this->ram = (uint8_t*)malloc(1024 * 1024);
  if(!this->ram) { fclose(rom_file); throw "error allocating rosco ram"; }

  this->rom = (uint8_t*)malloc(1024 * 1024);
  if(!this->rom) { fclose(rom_file); free(this->ram); throw "error allocating rosco rom"; }

  size_t rom_bytes_wrote = 0;
  size_t rom_bytes_read = fread(this->rom, 1, 1024, rom_file);
  while(rom_bytes_read > 0) {
    rom_bytes_wrote += rom_bytes_read;
    rom_bytes_read = fread(this->rom + rom_bytes_wrote, 1, 1024, rom_file);
  }
  fclose(rom_file);

  this->interrupt_controller = new InterruptController();
  this->duart = new Duart68681();
  this->interrupt_controller->sourceAdd(this->duart, 4); // TODO: what is the actual level?
}

RoscoM68K::~RoscoM68K() {
  if(this->ram) { free(this->ram); }
  if(this->rom) { free(this->rom); }
  delete this->interrupt_controller;
  delete this->duart;
}

void RoscoM68K::reset() {
  this->setModel(moira::Model::M68010);
  this->interrupt_controller->reset();

  // Moira will load up the stack and reset vectors from read* functions during a reset call
  // Rosco swaps ROM for RAM during the first four cycles
  // so, we'll just swap it around the reset
  uint8_t* swapped_ram = this->ram;
  this->ram = this->rom;
  moira::Moira::reset();
  this->ram = swapped_ram;
}

void RoscoM68K::getRegisters(m68k_registers* registers) {
  if(!registers) { return; }

  uint16_t status_register = ((uint16_t)this->reg.sr.ipl) << 8;
  if(this->reg.sr.c ) { status_register |= 0x0001; }
  if(this->reg.sr.v ) { status_register |= 0x0002; }
  if(this->reg.sr.z ) { status_register |= 0x0004; }
  if(this->reg.sr.n ) { status_register |= 0x0008; }
  if(this->reg.sr.x ) { status_register |= 0x0010; }
  if(this->reg.sr.s ) { status_register |= 0x0100; }
  if(this->reg.sr.t1) { status_register |= 0x8000; }

  registers->pc  = this->reg.pc;
  registers->usp = this->reg.usp;
  registers->ssp = this->reg.isp;
  registers->vbr = this->reg.vbr;
  registers->sr  = status_register;
  for(uint8_t idx=0; idx<8; ++idx) {
    registers->d[idx] = this->reg.d[idx];
    registers->a[idx] = this->reg.a[idx];
  }
}

void RoscoM68K::run(uint32_t cycle_count) {
  while(cycle_count) {
    this->setIPL(this->interrupt_controller->mpuPollInterrupt());
    this->execute();
    --cycle_count;
  }
}

uint8_t RoscoM68K::read8(uint32_t address) {
  if(address < 0x100000) { return this->ram[address]; }            // On-board RAM ( 1 MiB)
  if(address < 0xE00000) { return 0x00; }                          // empty space  (13 MiB)
  if(address < 0xF00000) { return this->rom[address - 0xE00000]; } // On-board ROM ( 1 MiB)
  if(address < 0xFFFFFF) {                                         // I/O Space    ( 1 MiB)
    if(address < 0xF00020) {
      if(address & 1) {
        // FF_00_00-FF_00_20 @ odd == DUART
        address = ((address - 0xF00000) >> 1) & 0xFF;
        return this->duart->read((uint8_t)address);
      }
    }
    return 0x00;
  }
  return 0x00;
}

uint16_t RoscoM68K::read16(uint32_t address) {
  uint16_t byte_high = this->read8(address);
  uint16_t byte_low  = this->read8(address + 1);
  return (byte_high << 8) | byte_low;
}

void RoscoM68K::write8(uint32_t address, uint8_t value) {
  if(address < 0x100000) { this->ram[address] = value; } // On-board RAM ( 1 MiB)
  if(address < 0xE00000) { return; }                     // empty space  (13 MiB)
  if(address < 0xF00000) { return; }                     // On-board ROM ( 1 MiB)
  if(address < 0xFFFFFF) {                               // I/O Space    ( 1 MiB)
    if(address < 0xF00020) {
      if(address & 1) {
        // FF_00_00-FF_00_20 @ odd == DUART
        address = ((address - 0xF00000) >> 1) & 0xFF;
        this->duart->write((uint8_t)address, value);
      }
    }
    return;
  }
  return;
}

void RoscoM68K::write16(uint32_t address, uint16_t value) {
  uint8_t byte_high = (uint8_t)(value >> 8);
  uint8_t byte_low  = (uint8_t)(value & 0xFF);
  this->write8(address+0, byte_high);
  this->write8(address+1, byte_low);
}

uint16_t RoscoM68K::readIrqUserVector(uint8_t level) const {
  return (uint16_t)(this->interrupt_controller->mpuReadVector(level));
}

void RoscoM68K::addressExtentsRam(uint32_t* lowest, uint32_t* highest) {
  if(lowest ) { *lowest  = 0x000000; }
  if(highest) { *highest = 0x0FFFFF; }
}

void RoscoM68K::addressExtentsRom(uint32_t* lowest, uint32_t* highest) {
  if(lowest ) { *lowest  = 0xFC0000; }
  if(highest) { *highest = 0xFFFFFF; }
}

uint8_t RoscoM68K::busRead(uint32_t address) {
  return this->read8(address);
}

/*
Address Layout
  |  begin   |   end    |  range   |   size   |  description    |
  |----------|----------|----------|----------|-----------------|
  | 00_00_00 | 00_0F_FF | 00_10_00 |    4 KiB | Vector Table    |
  | 00_00_00 | 0F_FF_FF | 10_00_00 | 1024 KiB | On-board RAM    |
  | 10_00_00 | DF_FF_FF | D0_00_00 |   13 MiB |                 |
  | E0_00_00 | EF_FF_FF | 10_00_00 | 1024 KiB | On-board ROM    |
  | F0_00_00 | FF_FF_FF | 10_00_00 | 1024 KiB | I/O Space       |

I/O Layout
  |  begin   |   end    | odd/even |     description      | 
  |----------|----------|----------|----------------------|
  | F8_00_00 | F8_00_1F |    odd   |    68681 DUART       | // XR68C681P provides UART, Timers and SD Card/SPI/GPIO
*/
