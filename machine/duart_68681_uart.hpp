#pragma once

extern "C" {
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
}

typedef void (*serialTransmit)(uint8_t port, uint8_t transmit_data, void* callback_data);

#define UART_INTERRUPT_TX_READY 1
#define UART_INTERRUPT_RX_READY 2
#define UART_INTERRUPT_BREAK    4

class Duart68681Uart {
public:
  Duart68681Uart(uint8_t port_number);

  void setTransmitter(serialTransmit transmitter, void* callback_data);
  void receive(uint8_t data);

  void    reset();
  uint8_t pollForInterrupt();
  uint8_t busRead(uint8_t address);
  void    busWrite(uint8_t address, uint8_t data);

protected:
  uint8_t port_number;
  bool receiver_enabled;
  bool transmitter_enabled;

  serialTransmit transmitter_callback;
  void* transmitter_callback_data;

  uint8_t receive_buffer[255];
  uint8_t receive_buffer_index;
  uint8_t receive_buffer_length;
  pthread_mutex_t receive_buffer_mutex;

  uint8_t register_mode_index;
  uint8_t register_mode[2];
  uint8_t register_clock_select;
};
