#pragma once

extern "C" {
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
}
#include <moira/Moira.h>
#include "interrupt_controller.hpp"
#include "duart_68681.hpp"

typedef struct {
  uint32_t pc;
  uint16_t sr;
  uint32_t usp, ssp;
  uint32_t d[8];
  uint32_t a[8];
  uint32_t vbr;
} m68k_registers;

class RoscoM68K : public moira::Moira {
public:
  RoscoM68K(const char* rom_path);
  ~RoscoM68K();

  void getRegisters(m68k_registers* registers);
  void reset();
  void run(uint32_t cycle_count);

  void addressExtentsRam(uint32_t* lowest, uint32_t* highest);
  void addressExtentsRom(uint32_t* lowest, uint32_t* highest);
  uint8_t busRead(uint32_t address);

protected:
  uint8_t  read8  (uint32_t address) override;
  uint16_t read16 (uint32_t address) override;
  void     write8 (uint32_t address, uint8_t value) override;
  void     write16(uint32_t address, uint16_t value) override;
  uint16_t readIrqUserVector(uint8_t level) const override;

  uint8_t* ram;
  uint8_t* rom;
  InterruptController* interrupt_controller;
  Duart68681* duart;
};
