#include "duart_68681.hpp"
#define UNINITIALIZED_VECTOR 0x0F

Duart68681::Duart68681() : port_a(0), port_b(1) {
  this->standby_mode = false;
  this->reset();
}

Duart68681::~Duart68681() {
}

void Duart68681::reset() {
  this->interrupt_vector_register = UNINITIALIZED_VECTOR;
  this->interrupt_mask_regsiter   = 0x00;
  this->port_a.reset();
  this->port_b.reset();
  this->input_port_changes = this->input_port_value; // whatever is causing the input should reset itself too, and call setInputPort()
  this->auxiliary_control  = 0x00;
  this->counter_timer      = 0x0000;
  this->output_port        = 0x00;
}

void Duart68681::serialPortReceive(uint8_t port, uint8_t data) {
  if(this->standby_mode) { return; }

  // 68681 reads a byte from serial
  if(port == 0) { this->port_a.receive(data); }
  if(port == 1) { this->port_b.receive(data); }
}

void Duart68681::setSerialTransmitter(uint8_t port, serialTransmit transmitter, void* callback_data) {
  // where 68681 sends its serial data
  if(port == 0) { this->port_a.setTransmitter(transmitter, callback_data); }
  if(port == 1) { this->port_b.setTransmitter(transmitter, callback_data); }
}

uint8_t Duart68681::busRead(uint8_t address) {
  if(this->standby_mode) { return 0x00; }

  switch(address) {
    case 0x00: return this->port_a.busRead(address);
    case 0x01: return this->port_a.busRead(address);
    case 0x03: return this->port_a.busRead(address);
    case 0x08: return this->port_b.busRead(address);
    case 0x09: return this->port_b.busRead(address);
    case 0x0B: return this->port_b.busRead(address);

    case 0x02: return this->getIsr() & this->interrupt_mask_regsiter;
    case 0x04: {
      uint8_t ipcr = (this->input_port_value & 0xF) | ((this->input_port_changes & 0xF) << 4);
      this->input_port_changes = 0;
      return ipcr;
    }
    case 0x05: return this->getIsr();
    case 0x06: return (uint8_t)((this->counter_timer & 0xFF00) >> 8);
    case 0x07: return (uint8_t)(this->counter_timer & 0x00FF);
    case 0x0C: return this->interrupt_vector_register;
    case 0x0D: return this->input_port_value;
    case 0x0E: { this->timerStart(); return 0x00; }
    case 0x0F: { this->timerStop(); return 0x00; }
  }

  return 0x00;
}

void Duart68681::busWrite(uint8_t address, uint8_t data) {
  if(this->standby_mode) {
    // exit stand-by mode from a CRA/CRB command, with 0xD in upper nibble of data
    if((address == 0x02) || (address == 0x0A)) {
      if((data & 0xF0) == 0xD0) {
        this->standby_mode = false;
      }
    }
    return;
  }

  switch(address) {
    case 0x00: this->port_a.busWrite(address, data); break;
    case 0x01: this->port_a.busWrite(address, data); break;
    case 0x02: this->port_a.busWrite(address, data); break;
    case 0x03: this->port_a.busWrite(address, data); break;
    case 0x08: this->port_b.busWrite(address, data); break;
    case 0x09: this->port_b.busWrite(address, data); break;
    case 0x0A: this->port_b.busWrite(address, data); break;
    case 0x0B: this->port_b.busWrite(address, data); break;

    case 0x04: this->auxiliary_control = data; break;
    case 0x05: this->interrupt_mask_regsiter = data; break;
    case 0x06: this->counter_timer = (this->counter_timer & 0x00FF) | (((uint16_t)(data)) << 8);
    case 0x07: this->counter_timer = (this->counter_timer & 0xFF00) | ((uint16_t)(data));
    case 0x0C: this->interrupt_vector_register = data; break;
    case 0x0D: break; // TODO: output port configuration (OPCR); ignoring this for now
    case 0x0E: this->output_port |= data; break;
    case 0x0F: this->output_port &= ~data; break;
  }
}

uint8_t Duart68681::readVector() {
  return this->interrupt_vector_register;
}

uint8_t Duart68681::getIsr() {
  bool port_change_interrupt = ((this->input_port_changes & 0xF) & (this->auxiliary_control & 0xF)) > 0;

  uint8_t interrupt_bits = 0;
  interrupt_bits |= this->port_a.pollForInterrupt();            // bits 0,1,2
  interrupt_bits |= this->port_b.pollForInterrupt() << 2;       // bits 4,5,6
  interrupt_bits |= (port_change_interrupt ? 1 : 0) << 7;       // bit 7
  interrupt_bits |= (this->timerCheckInterrupt() ? 1 : 0) << 3; // bit 3

  return interrupt_bits;
}

bool Duart68681::pollForInterrupt() {
  if(this->standby_mode) { return false; }
  return (this->getIsr() & this->interrupt_mask_regsiter) > 0;
}

void Duart68681::setInputPort(uint8_t value) {
  value &= 0x3F;
  this->input_port_changes = this->input_port_value ^ value;
  this->input_port_value = value;
}

uint8_t Duart68681::readOutputPort() {
  return this->output_port;
}

void Duart68681::timerStart() {
  int timer_divider = 0;

  if((this->auxiliary_control & 0x70) == 0x60) {
    timer_divider = 1;
  } else if((this->auxiliary_control & 0x70) == 0x70) {
    timer_divider = 16;
  } else {
    // we only suport timer:xtal/1 and timer:xtal/16 ACR modes
    timer_started = false;
    timer_cleared = false;
    timer_interrupt_current = 0;
    timer_interrupt_next = 0;
    return;
  }

  if(this->counter_timer == 0) {
    // not running at constant interrupt
    timer_started = false;
    timer_cleared = false;
    timer_interrupt_current = 0;
    timer_interrupt_next = 0;
    return;
  }

  double f_base         = 3686400.0;
  double f_divisor      = (double)timer_divider;
  double f_counter      = (double)this->counter_timer;
  double f_frequency    = (f_base / f_divisor) / (2.0 * f_counter);
  double f_milliseconds = 1000.0 / f_frequency;

  time_t now = time(NULL);
  this->timer_interval          = (time_t)f_milliseconds;
  this->timer_started           = true;
  this->timer_cleared           = false;
  this->timer_interrupt_current = now + this->timer_interval;
  this->timer_interrupt_next    = this->timer_interrupt_current + this->timer_interval;
}

void Duart68681::timerStop() {
  this->timer_cleared = true;
}

bool Duart68681::timerCheckInterrupt() {
  if(!timer_started) { return false; }

  time_t now = time(NULL);

  if(now >= this->timer_interrupt_next) {
    this->timer_cleared           = false;
    this->timer_interrupt_current = this->timer_interrupt_next;
    this->timer_interrupt_next    = now + this->timer_interval;
    return true;
  }

  if(now >= this->timer_interrupt_current) {
    // if(this->timer_cleared == false) {
    //   this->timer_cleared = true;
    //   return true;
    // } else {
    //   return false;
    // }
    return !this->timer_cleared;
  }

  return false;
}
