#include "duart_68681_uart.hpp"

Duart68681Uart::Duart68681Uart(uint8_t port_number) {
  this->port_number = port_number;
}

void Duart68681Uart::reset() {
  this->receiver_enabled      = false;
  this->transmitter_enabled   = false;
  this->receive_buffer_index  = 0;
  this->receive_buffer_length = 0;
  pthread_mutex_init(&(this->receive_buffer_mutex), NULL);
  this->register_mode_index   = 0;
  this->register_mode[0]      = 0; // TODO default value?
  this->register_mode[1]      = 0; // TODO default value?
  this->register_clock_select = 0; // TODO default value?
}

void Duart68681Uart::setTransmitter(serialTransmit transmitter, void* callback_data) {
  this->transmitter_callback = transmitter;
  this->transmitter_callback_data = callback_data;
}

void Duart68681Uart::receive(uint8_t data) {
  if(this->receive_buffer_length == 0xFF) { return; } // overflow

  pthread_mutex_lock(&(this->receive_buffer_mutex));
  if(this->receiver_enabled == false) { // disabled
    pthread_mutex_unlock(&(this->receive_buffer_mutex));
    return;
  }
  uint8_t free_index = this->receive_buffer_index + this->receive_buffer_length;
  this->receive_buffer[free_index] = data;
  ++(this->receive_buffer_length);
  pthread_mutex_unlock(&(this->receive_buffer_mutex));
}

uint8_t Duart68681Uart::pollForInterrupt() {
  uint8_t bits = this->transmitter_enabled ? UART_INTERRUPT_TX_READY : 0;
  // TODO: should really differentiate between RxRDY & RxFULL here...
  if(this->receive_buffer_length) { bits |= UART_INTERRUPT_RX_READY; }
  return bits;
}

uint8_t Duart68681Uart::busRead(uint8_t address) {
  if(this->port_number) { address -= 8; } // offset for PortB

  switch(address) {
    case 0x00: { // mode
      uint8_t mode = this->register_mode[this->register_mode_index];
      this->register_mode_index = 1;
      return mode;
    }
    case 0x01: { // status register
      uint8_t status = 0;
      if(this->transmitter_enabled)       { status |= 0xC; }
      if(this->receive_buffer_length > 2) { status |= 0x2; }
      if(this->receive_buffer_length > 0) { status |= 0x1; }
      return status;
    }
    /* status register
      bit 7 - received break -- don't care
      bit 6 - framing error -- don't care
      bit 5 - partity error -- don't care
      bit 4 - overrun error -- don't care
      bit 3 - transmitter empty -- always true if transmitter enabled (because we send immediately)
      bit 2 - transmitter ready -- functionally same as bit 3, for us
      bit 1 - receiver FIFO full -- true if receive_buffer_length > 2
      bit 0 - receiver ready -- true if receive_buffer_length > 0
    */

    case 0x03: { // receive holding
      if(!this->receive_buffer_length) {
        return 0;
      }
      pthread_mutex_lock(&(this->receive_buffer_mutex));
      uint8_t received_data = this->receive_buffer[this->receive_buffer_index];
      ++(this->receive_buffer_index);
      --(this->receive_buffer_length);
      pthread_mutex_unlock(&(this->receive_buffer_mutex));
      return received_data;
    }
  }

  return 0;
}

void Duart68681Uart::busWrite(uint8_t address, uint8_t data) {
  if(this->port_number) { address -= 8; } // offset for PortB

  switch(address) {
    case 0x00: { // mode
      this->register_mode[this->register_mode_index] = data;
      this->register_mode_index = 1;
      return;
    }
    /*
      mode register 1 (MR1A/MR1B)
      bit 7 - Receiver RTS Control -- don't care
      bit 6 - Receiver Interrupt Type (0:Rx data > 0, 1:Rx data full (>=3))
      bit 5 - Error mode select (0: character, 1: block) -- don't care
      bit 4,3 - Parity mode select -- don't care
      bit 2 - parity type -- don't care
      bit 1,0 - number of bits per character (0:5, 1:6, 2:7, 3:8) -- should always be 7 or 8; probably safe to ignore

      mode register 2 (MR2A/MR2B)
      bit 7,6 - channel mode (0:normal, 1:echo, 2:local loop, 3: remote loop) -- maybe care?
      bit 5 - Tx RTS Control -- don't care
      bit 4 - CTS enabled Tx -- don't care
      bit 3,2,1,0 - stop bit length -- don't care
    */

    case 0x01: { // clock select
      this->register_clock_select = data;
      return;
    }

    case 0x02: { // command
      if((data & 0x03) == 0x01) { this->receiver_enabled    = true;  }
      if((data & 0x03) == 0x02) { this->receiver_enabled    = false; }
      if((data & 0x0C) == 0x04) { this->transmitter_enabled = true;  }
      if((data & 0x0C) == 0x08) { this->transmitter_enabled = false; }

      uint8_t nibble_upper = data & 0xF0;
      switch(nibble_upper) {
        case 0x10: { this->register_mode_index = 0; } break;
        case 0x20: {
          pthread_mutex_lock(&(this->receive_buffer_mutex));
          this->receiver_enabled      = false;
          this->receive_buffer_index  = 0;
          this->receive_buffer_length = 0;
          pthread_mutex_unlock(&(this->receive_buffer_mutex));
          break;
        }
        case 0x40: { /* ignoring command: clear error flags in status register */ } break;
        case 0x50: { /* this->interrupt_bits &= ~UART_INTERRUPT_BREAK; */ /*  NOTE: ignoring break interrupts */ } break;
      }

      /*
        0000  no op
        0001  reset MRn pointer (to MR1n)
        0010  reset receiver (receiver is disabled, and fifo is emptied)
        0011  reset transmitter (TXDn set high)
        0100  reset error status (clear error flags in status register)
        0101  reset break change interrupt (clear break change interrupt status flag)
        0110  start break (forces TXDn low; after THR is emptied)
        0111  stop break (TXDn goes high; transmission can resume)
        1000  set Rx BRG select extend bit (???)
        1001  clear Rx BRG select extend bit (???)
        1010  set Tx BRG select extend bit (???)
        1011  clear Tx BRG select extend bit (???)
        1100  set standby mode (sets chip to standby mode (both uarts, and gpio))
        1101  set active mode (resumes operation after entering standby mode)
        1110  (reserved)
        1111  (reserved)
      */

      return;
    }

    case 0x03: { // transmit holding
      if(this->transmitter_enabled) {
        if(this->transmitter_callback) {
          this->transmitter_callback(this->port_number, data, this->transmitter_callback_data);
        }
      }
      return;
    }
  }
}
