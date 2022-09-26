#pragma once

extern "C" {
#include <stdint.h>
#include <stdbool.h>
}

class InterruptSource {
public:
  virtual void reset() = 0;
  virtual uint8_t readVector() = 0;
  virtual bool pollForInterrupt() = 0;
};
