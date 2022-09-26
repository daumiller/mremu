#include "duart_68681.hpp"
#define UNINITIALIZED_VECTOR 0x0F

Duart68681::Duart68681() {
  this->reset();
}

Duart68681::~Duart68681() {
}

uint8_t Duart68681::read(uint8_t address) {
  return 0x00;
}

void Duart68681::write(uint8_t address, uint8_t data) {
}

void Duart68681::reset() {
  this->interrupt_request = false;
  this->interrupt_vector_register = UNINITIALIZED_VECTOR;
}

uint8_t Duart68681::readVector() {
  return this->interrupt_vector_register;
}

bool Duart68681::pollForInterrupt() {
  return this->interrupt_request;
}
