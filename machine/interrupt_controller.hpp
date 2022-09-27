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

/**
 * Interrupt Controller
 **/
class InterruptController {
public:
  InterruptController();

  /**
   * Add an interrupt source to this controller
   * 
   * @param source interrupt source
   * @param level  level of interrupt; can think of this as interrupt number, since levels aren't shared
   * @returns whether the source was successfully added; will fail if another device is already using the given level
   */
  bool sourceAdd(InterruptSource* source, uint8_t level);

  /**
   * Remove an interrupt source from this controller
   * 
   * @param source interrupt source device to remove
   * @returns whether device was removed (was found)
   */
  bool sourceRemove (InterruptSource* source);

  /**
   * Disable an interrupt source
   * NOTE: devices are enabled be default, when added to the controller
   * 
   * @param source interrupt source to disable
   * @returns whether device was disabled (was found)
   */
  bool sourceDisable(InterruptSource* source);

  /**
   * Enable an interrupt source
   * NOTE: devices are enabled be default, when added to the controller
   * 
   * @param source interrupt source to enable
   * @returns whether device was enabled (was found)
   */
  bool sourceEnable (InterruptSource* source);

  /**
   * Remove an interrupt source
   * 
   * @param level interrupt level (number) of device to remove
   * @returns whether device was removed (was found)
   */
  bool sourceRemove (uint8_t level);

  /**
   * Disable an interrupt source
   * NOTE: devices are enabled be default, when added to the controller
   * 
   * @param level interrupt level (number) of device to remove
   * @returns whether device was disabled (was found)
   */
  bool sourceDisable(uint8_t level);

  /**
   * Enable an interrupt source
   * NOTE: devices are enabled be default, when added to the controller
   * 
   * @param level interrupt level (number) of device to remove
   * @returns whether device was enabled (was found)
   */
  bool sourceEnable (uint8_t level);

  /**
   * Poll called by the processor, to determine if an interrupt has been request
   * 
   * @returns current reported interrupt level (IPL0,IPL1,IPL2)
   */
  uint8_t mpuPollInterrupt();

  /**
   * Read called by the processor, to get the vector of the current interrupt
   * 
   * @returns vector read from interrupt source at given level
   */
  uint8_t mpuReadVector(uint8_t level);

  /**
   * Called by processor, to reset this controller and all of its connected sources
   */
  void reset();

private:
  InterruptSource* source[7];
  bool source_enabled[7];
};
