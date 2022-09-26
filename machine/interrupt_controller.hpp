#pragma once

/*
  Interrupt Controller
    accepts individual interrupt signals from sources
    encodes those interrupts with a priority number (level)
    mpu polls IPL with mpuPollInterrupt()
    mpu reads interrupt vector with mpuReadVector()

    level 0 is invalid as a source
    levels 1-6 are maskable interrupts (may be ignored by MPU)
    level 7 is non-maskable
*/

#include "interrupt_source.hpp"

class InterruptController {
public:
  InterruptController();

  bool sourceAdd(InterruptSource* source, uint8_t level);
  bool sourceRemove (InterruptSource* source);
  bool sourceDisable(InterruptSource* source);
  bool sourceEnable (InterruptSource* source);
  bool sourceRemove (uint8_t level);
  bool sourceDisable(uint8_t level);
  bool sourceEnable (uint8_t level);

  uint8_t mpuPollInterrupt();
  uint8_t mpuReadVector(uint8_t level);
  void reset();

private:
  InterruptSource* source[7];
  bool source_enabled[7];
};
