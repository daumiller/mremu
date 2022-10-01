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
/**
 * structure for inspecting 68K registers
 **/
typedef struct {
  uint32_t pc;        // program counter
  uint16_t sr;        // status register
  uint32_t usp;       // user stack pointer
  uint32_t ssp;       // supervisor stack pointer
  uint32_t d[8];      // data registers
  uint32_t a[8];      // address registers; A7 == current stack pointer
  uint32_t vbr;       // vector base register
} m68k_registers;

/**
 * Instance of a rosco-m68k Classic V2 single-board computer
 **/
class RoscoM68K : public moira::Moira {
public:
  /**
   * Create a new rosco-m68k instance
   * 
   * @param rom_path path to a file to load as ROM (limit 1 MiB)
   **/
  RoscoM68K(const char* rom_path);
  ~RoscoM68K();

  /**
   * Copy current register values
   * 
   * @param registers pointer to an m68k_registers structure to fill
   **/
  void getRegisters(m68k_registers* registers);

  /**
   * Reset this rosco-m68k, and its peripherals
   **/
  void reset();

  /**
   * Run processor cycle(s)
   * 
   * @param cycle_count count of processor cycles to run
   **/
  void run(uint32_t cycle_count);

  /**
   * Get extents of RAM, in bus addresses
   * 
   * @param lowest receives lowest address in RAM
   * @param highest receives highest address in RAM
   **/
  void addressExtentsRam(uint32_t* lowest, uint32_t* highest);
  /**
   * Get extents of ROM, in bus addresses
   * 
   * @param lowest receives lowest address in ROM
   * @param highest receives highest address in ROM
   **/
  void addressExtentsRom(uint32_t* lowest, uint32_t* highest);
  /**
   * Read data byte from bus
   * 
   * @param address address to read from
   * @returns byte read from bus
   **/
  uint8_t busRead(uint32_t address);

  // internal state
  uint8_t* ram;
  uint8_t* rom;
  InterruptController* interrupt_controller;
  Duart68681* duart;

protected:
  // Moira overrides for bus accesses, and forward IRQ related things to our interrupt controller
  uint8_t  read8  (uint32_t address) override;
  uint16_t read16 (uint32_t address) override;
  void     write8 (uint32_t address, uint8_t value) override;
  void     write16(uint32_t address, uint16_t value) override;
  uint16_t readIrqUserVector(uint8_t level) const override;
};

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
  | F0_00_00 | F0_00_1F |    odd   |    68681 DUART       | // XR68C681P provides UART, Timers and SD Card/SPI/GPIO

Address mapping at boot:
  GAL IC 5 Address Decoder creates a BOOT signal, that activates after the first 4 clock cycles after reset,
  this signal also disables ram/enables rom for reading the first two vectors from ROM addresses instead of RAM
*/
