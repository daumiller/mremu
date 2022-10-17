#include "duart.h"

// =============================================================================
//   Serial
// =============================================================================
void duartSerial_writeCharacter(duart_serial_port port, uint8_t character) {
  volatile uint8_t* port_status   = (port == DUART_SERIAL_A) ? DUART_A_STATUS   : DUART_B_STATUS;
  volatile uint8_t* port_transmit = (port == DUART_SERIAL_A) ? DUART_A_TRANSMIT : DUART_B_TRANSMIT;
  while(duartTransferReady(port_status) == 0) {}
  *port_transmit = character;
}

void duartSerial_writeString(duart_serial_port port, const uint8_t* string) {
  volatile uint8_t* port_status   = (port == DUART_SERIAL_A) ? DUART_A_STATUS   : DUART_B_STATUS;
  volatile uint8_t* port_transmit = (port == DUART_SERIAL_A) ? DUART_A_TRANSMIT : DUART_B_TRANSMIT;
  while(string[0]) {
    while(duartTransferReady(port_status) == 0) {}
    *port_transmit = string[0];
    ++string;
  }
}

void duartSerial_writeBuffer(duart_serial_port port, const uint8_t* buffer, uint32_t buffer_size) {
  volatile uint8_t* port_status   = (port == DUART_SERIAL_A) ? DUART_A_STATUS   : DUART_B_STATUS;
  volatile uint8_t* port_transmit = (port == DUART_SERIAL_A) ? DUART_A_TRANSMIT : DUART_B_TRANSMIT;
  while(buffer_size) {
    while(duartTransferReady(port_status) == 0) {}
    *port_transmit = buffer[0];
    ++buffer;
    --buffer_size;
  }
}

uint8_t duartSerial_readCharacter(duart_serial_port port) {
  volatile uint8_t* port_status  = (port == DUART_SERIAL_A) ? DUART_A_STATUS   : DUART_B_STATUS;
  volatile uint8_t* port_receive = (port == DUART_SERIAL_A) ? DUART_A_TRANSMIT : DUART_B_TRANSMIT;
  while(duartReceiverReady(port_status) == 0) {}
  return *port_receive;
}

bool duartSerial_readLine(duart_serial_port port, uint8_t* buffer, uint32_t buffer_maximum_size, uint32_t* optional_read_length) {
  volatile uint8_t* port_status  = (port == DUART_SERIAL_A) ? DUART_A_STATUS   : DUART_B_STATUS;
  volatile uint8_t* port_receive = (port == DUART_SERIAL_A) ? DUART_A_TRANSMIT : DUART_B_TRANSMIT;

  uint8_t  read_byte;
  uint32_t read_length = 0;
  --buffer_maximum_size; // don't count null-terminator byte

  while(read_length < buffer_maximum_size) {
    while(duartReceiverReady(port_status) == 0) {}
    read_byte = *port_receive;
    if(read_byte == '\r') { continue; }
    if(read_byte == '\n') {
      buffer[read_length] = 0x00;
      if(optional_read_length) { *optional_read_length = read_length; }
      return true;
    }
    buffer[read_length] = read_byte;
    ++read_length;
  }

  buffer[read_length] = 0x00;
  if(optional_read_length) { *optional_read_length = read_length; }
  return false;
}

void duartSerial_readBuffer(duart_serial_port port, uint8_t* buffer, uint32_t buffer_size) {
  volatile uint8_t* port_status  = (port == DUART_SERIAL_A) ? DUART_A_STATUS   : DUART_B_STATUS;
  volatile uint8_t* port_receive = (port == DUART_SERIAL_A) ? DUART_A_TRANSMIT : DUART_B_TRANSMIT;
  while(buffer_size) {
    while(duartReceiverReady(port_status) == 0) {}
    buffer[0] = *port_receive;
    ++buffer;
    --buffer_size;
  }
}

// =============================================================================
//   Output
// =============================================================================
void duartOutput_enable() {
  // unfortunately, DUART_OUTPUT_CONFIGURATION is a write-only register,
  // so we can't pick and choose values. we'll just enable all outputs.
  *DUART_OUTPUT_CONFIGURATION = 0x00;
}

// =============================================================================
//   LEDs
// =============================================================================
void duartLed_set(duart_led led, bool on) {
  uint8_t bits = (led == DUART_LED_GREEN) ? DUART_OUTPUT_LED_GREEN_INVERSE : DUART_OUTPUT_LED_RED_INERSE;
  if(on) {
    duartOutputLow(bits);
  } else {
    duartOutputHigh(bits);
  }
}

// =============================================================================
//   SPI
// =============================================================================
void duartSpi_select(bool spi_a_selected, bool spi_b_selected) {
  uint8_t bits_high = 0x00;
  uint8_t bits_low  = 0x00;
  if(spi_a_selected) { bits_low |= DUART_OUTPUT_SPI_CSA_INVERSE; } else { bits_high |= DUART_OUTPUT_SPI_CSA_INVERSE; }
  if(spi_b_selected) { bits_low |= DUART_OUTPUT_SPI_CSB_INVERSE; } else { bits_high |= DUART_OUTPUT_SPI_CSB_INVERSE; }
  duartOutputHigh(bits_high); // output deactivations first,
  duartOutputLow(bits_low);   // then any activations
}

uint8_t duartSpi_transferByte(uint8_t send_byte) {
  uint8_t bits_high = 0x00;
  uint8_t bits_low  = 0x00;
  uint8_t read_byte = 0x00;
  for(uint8_t index=0; index<8; ++index) {
    // shift read_byte value; doesn't matter on first iteration
    read_byte <<= 1;

    // prepare clock (high) and output bit values
    bits_high = DUART_OUTPUT_SPI_CLOCK;
    bits_low  = 0x00;
    if(send_byte & 0x80) {
      bits_high |= DUART_OUTPUT_SPI_COPI;
    } else {
      bits_low |= DUART_OUTPUT_SPI_COPI;
    }

    // set output put, then (or simultaneously), set clock high
    duartOutputLow(bits_low);
    duartOutputHigh(bits_high);
    // try to even out duty cycle
    for(uint8_t delay=0; delay<2; ++delay) {}

    // read input bit
    if(duartInputRead(DUART_INPUT_CIPO)) {
      read_byte |= 1;
    }
    // shift next bit into MSB of sending byte (here for more clock high time)
    send_byte <<= 1;
    
    // set clock low, wait a bit
    duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
  }

  return read_byte;
}

void duartSpi_readBuffer(uint8_t* receive_buffer, uint32_t length) {
  while(length) {
    receive_buffer[0] = duartSpi_transferByte(0xFF);
    ++receive_buffer;
    --length;
  }
}

void duartSpi_writeBuffer(uint8_t* send_buffer, uint32_t length) {
  while(length) {
    duartSpi_transferByte(send_buffer[0]);
    ++send_buffer;
    --length;
  }
}

void duartSpi_transferBuffer(uint8_t* send_buffer, uint8_t* receive_buffer, uint32_t length) {
  while(length) {
    receive_buffer[0] = duartSpi_transferByte(send_buffer[0]);
    ++send_buffer;
    ++receive_buffer;
    --length;
  }
}
