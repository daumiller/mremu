extern "C" {
#include <stdlib.h>
}
#include "interrupt_controller.hpp"
#define UNINITIALIZED_VECTOR 0x0F

InterruptController::InterruptController() {
  for(int idx=0; idx<7; ++idx) {
    this->source[idx] = NULL;
    this->source_enabled[idx] = false;
  }
}

void InterruptController::reset() {
  for(int idx=0; idx<7; ++idx) {
    if(this->source[idx]) {
      this->source[idx]->reset();
    }
  }
}

bool InterruptController::sourceAdd(InterruptSource* source, uint8_t level) {
  if(level < 1) { return false; }
  if(level > 7) { return false; }
  uint8_t index = level - 1;
  if(this->source[index]) { return false; }
  this->source[index] = source;
  this->source_enabled[index] = true;
  return true;
}

bool InterruptController::sourceRemove(InterruptSource* source) {
  for(int index=0; index<7; ++index) {
    if(this->source[index] == source) {
      this->source[index] = NULL;
      this->source_enabled[index] = false;
      return true;
    }
  }
  return false;
}

bool InterruptController::sourceRemove(uint8_t level) {
  if(level < 1) { return false; }
  if(level > 7) { return false; }
  uint8_t index = level - 1;
  if(this->source[index]) {
    this->source[index] = NULL;
    this->source_enabled[index] = false;
    return true;
  }
  return false;
}

bool InterruptController::sourceDisable(InterruptSource* source) {
  for(int index=0; index<7; ++index) {
    if(this->source[index] == source) {
      this->source_enabled[index] = false;
      return true;
    }
  }
  return false;
}

bool InterruptController::sourceDisable(uint8_t level) {
  if(level < 1) { return false; }
  if(level > 7) { return false; }
  uint8_t index = level - 1;
  if(this->source[index]) {
    this->source_enabled[index] = false;
    return true;
  }
  return false;
}

bool InterruptController::sourceEnable(InterruptSource* source) {
  for(int index=0; index<7; ++index) {
    if(this->source[index] == source) {
      this->source_enabled[index] = true;
      return true;
    }
  }
  return false;
}

bool InterruptController::sourceEnable(uint8_t level) {
  if(level < 1) { return false; }
  if(level > 7) { return false; }
  uint8_t index = level - 1;
  if(this->source[index]) {
    this->source_enabled[index] = true;
    return true;
  }
  return false;
}

uint8_t InterruptController::mpuPollInterrupt() {
  uint8_t interrupt_level = 0;

  for(uint8_t index=0; index<7; ++index) {
    if(this->source[index] && this->source_enabled[index]) {
      if(this->source[index]->pollForInterrupt()) {
        // no need to compare to our previous value, as level rises with each index
        interrupt_level = index + 1;
      }
    }
  }

  return interrupt_level;
}

uint8_t InterruptController::mpuReadVector(uint8_t level) {
  if(level < 1) { return UNINITIALIZED_VECTOR; }
  if(level > 7) { return UNINITIALIZED_VECTOR; }
  uint8_t index = level - 1;
  if(this->source[index] && this->source_enabled[index]) {
    return this->source[index]->readVector();
  }
  return UNINITIALIZED_VECTOR;
}
