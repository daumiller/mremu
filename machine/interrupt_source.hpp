#pragma once

extern "C" {
#include <stdint.h>
#include <stdbool.h>
}

class InterruptSource {
public:
  /**
   * reset interrupt source device
   */
  virtual void reset() = 0;

  /**
   * read interrupt vector from source device
   * @returns interrupt vector
   */
  virtual uint8_t readVector() = 0;

  /**
   * check if device is requesting interrupt
   * @returns whether device is requesting interrupt
   */
  virtual bool pollForInterrupt() = 0;
};
