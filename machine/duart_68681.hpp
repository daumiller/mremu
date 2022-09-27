#pragma once

extern "C" {
#include <stdint.h>
#include <stdbool.h>
}
#include "interrupt_source.hpp"
#include "duart_68681_uart.hpp"

#define DUART_68681_PORT_A 0
#define DUART_68681_PORT_B 1

/// @brief callback function used by serial ports to transmit data
typedef void (*serialTransmit)(uint8_t port, uint8_t transmit_data, void* callback_data);

/**
 * XC68C681 Dual UART Controller (plus Counter/Timer & GPIO)
 */
class Duart68681 : public InterruptSource {
public:
  Duart68681();
  ~Duart68681();

  /**
   * Read data byte from bus
   * @param address relative address to read from
   * @returns data byte read
   */
  uint8_t busRead(uint8_t address);

  /** Write data byte to bus address
   * @param address relative address to write to
   * @param data data byte to write
   */
  void busWrite(uint8_t address, uint8_t data);

  /**
   * Receive data on serial port
   * @param port port to receive on (DUART_68681_PORT_A or DUART_68681_PORT_B)
   * @param data data byte to receive
   */
  void serialPortReceive(uint8_t port, uint8_t data);

  /**
   * Set transmitter callback for serial port
   * @param port port to receive on (DUART_68681_PORT_A or DUART_68681_PORT_B)
   * @param transmitter callback to use for this port
   * @param callback_data extra data to pass when callback is called
   */
  void setSerialTransmitter(uint8_t port, serialTransmit transmitter, void* callback_data);

  /**
   * Set input port values
   * @param value 6 bit value (0-63) to set on input port
   */
  void setInputPort(uint8_t value);

  /**
   * Read output port values
   * @returns value 8 bit output port value
   */
  uint8_t readOutputPort();

  // InterrupSource implementation
  void    reset() override;
  uint8_t readVector() override;
  bool    pollForInterrupt() override;

protected:
  bool    standby_mode;
  uint8_t interrupt_vector_register;
  uint8_t interrupt_mask_regsiter;
  uint8_t getIsr();

  Duart68681Uart port_a;
  Duart68681Uart port_b;

  uint8_t input_port_value;
  uint8_t input_port_changes;
  uint8_t auxiliary_control;

  uint16_t counter_timer;
  uint8_t output_port;
};

// TODO: add SPI interface

/*
  XR68C681 Dual UART + GPIO + Counter/Timer
  (MC68681 compatible)

  Interface
    8 bit data
    4 bit address
    Operational /R/W, /CS, /DTACK
    Interrupts  /IACK, /INTR
    Interrupt Vector Output on IACK
  GPIO
    6 bit input port (IP0-IP5) (wich change triggers)
    8 bit output port (OP0-OP7)
  Timer
    16 bit counter/timer

  Per-UART-channel Registers
    Command Register (CRA, CRB)
    Mode Registers (MR1A, MR2A, MR1B, MR2B)
    Status Register (SRA, SRB)
    Clock Select Register (CSRA, CSRB)
    Receiver Holding Register (RHRA, RHRB)
    Transmit Holding Register (THRA, THRB)
  Both-channel Registers
    Interrupt Status Register (ISR)
    Interrupt Mask Register (IMR)
    Masked Interrupt Status Register (MISR)
    Interrupt Vector Register (IVR)
    Auxiliary control Register (ACR)
  GPIO/Timer Registers
    Output Port Control Register (OPCR)
    Input Port configuration Register (IPCR)
    Counter/Timer Upper Byte Register (CTUR)
    Counter/Timer Lower Byte Register (CTLR)
    Output Port Register (OPR)

  Auxiliary Control Register
    lower nibble controls whether input port pins trigger interrupts
    bits 4-6 used by counter/timer
    bit 7 determines which set of baud rates to use

  Crystal clock source setup between X2 & X1/CLK pins.
    assuming this is 3.6864 MHz (until my board arrives to verify)

  Counter/Timer
    16 bit down-counter
    ACR bits 4-6 determine counter/timer mode
      | 6 5 4 | mode    | timing source                              |
      | 0 0 0 | counter | external input IP2                         |
      | 0 0 1 | counter | 1x clock of chA transmitter ( == chA rate) |
      | 0 1 0 | counter | 1x clock of chB transmitter ( == chB baud) |
      | 1 0 0 | timer   | external input IP2                         |
      | 1 0 1 | timer   | external input IP2, divided by 16          |
      | 1 1 0 | timer   | xtal input                                 |
      | 1 1 1 | timer   | xtal input, divided by 16                  |
    Timer Frequency
      source_freq / (2 * ((CTUR << 8) | CTLR))
    Baud Rate
      source_freq / (32 * ((CTUR << 8) | CTLR))

  Input Port
    IP0 - CTSA (serial port A signal, clear to send)
    IP1 - CTSB (serial port B signal, clear to send)
    IP2 - SPIMISO
    IP3 - GND
    IP4 - n/c
    IP5 - n/c
  Output Port
    OP0 - RTSA (serial port A signal, ready to send)
    OP1 - RTSB (serial port B signal, ready to send)
    OP2 - SPICS
    OP3 - red LED
    OP4 - SPICLK
    OP5 - green LED
    OP6 - SPIMOSI
    OP7 - SPICS2
  SPI
    CS1  output:2
    CS2  output:7
    CLK  output:4
    MOSI output:6
    MISO input:2

  Register Address Map
  | Address | Mode  | Name       | Description               |
  |---------|-------|------------|---------------------------|
  |   00    | read  | MR1A, MR2A | mode channel A            | MRn points to MR1n @ reset, but switches to MR2n after any read/write
  |   00    | write | MR1A, MR2A | mode channel A            | can be reset to MR1n by a "RESET MR POINTER" command
  |   01    | read  |     SRA    | status channel A          |
  |   01    | write |    CSRA    | clock select channel A    |
  |   02    | read  |    MISR    | masked interrupt status   |
  |   02    | write |     CRA    | command channel A         |
  |   03    | read  |    RHRA    | rx holding channel A      |
  |   03    | write |    THRA    | tx holding channel A      |
  |   04    | read  |    IPCR    | input port change         |
  |   04    | write |     ACR    | auxiliary control         |
  |   05    | read  |     ISR    | interrupt status          |
  |   05    | write |     IMR    | interrupt mask            |
  |   06    | read  |     CTU    | counter/timer upper byte  |
  |   06    | write |     CTU    | counter/timer upper byte  |
  |   07    | read  |     CTL    | counter/timer lower byte  |
  |   07    | write |     CTL    | counter/timer lower byte  |
  |   08    | read  | MR1B, MR2B | mode channel B            |
  |   08    | write | MR1B, MR2B | mode channel B            |
  |   09    | read  |     SRB    | status channel B          |
  |   09    | write |    CSRB    | clock select channel B    |
  |   0a    | read  |            |                           |
  |   0a    | write |     CRB    | command channel B         |
  |   0b    | read  |    RHBB    | rx holding channel B      |
  |   0b    | write |    THRB    | tx holding channel B      |
  |   0c    | read  |     IVR    | interrupt vector          |
  |   0c    | write |     IVR    | interrupt vector          |
  |   0d    | read  |     IP     | input port                |
  |   0d    | write |    OPCR    | output port configuration |
  |   0e    | read  |     SCC    | start counter/timer       | * not r/w, but address-triggered
  |   0e    | write |   SOPBC    | set output port bits      | * not r/w, but address-triggered (also, output pins are ~value_wrote?)
  |   0f    | read  |     STC    | stop counter/timer        | * not r/w, but address-triggered
  |   0f    | write |   COPBC    | clear output port bits    | * not r/w, but address-triggered (also, output pins are ~value_wrote?)

  CRA/CRB
    lower nibble function map
      |       bit3 bit2     |       bit1 bit0     |
      | 00  no change to Tx | 00  no change to Rx |
      | 01  enable Tx       | 01  enable Rx       |
      | 10  disable Tx      | 10  disable Rx      |
      | 11  (reserved)      | 11  (reserved)      |
    upper nibble function map
      bit7 bit6 bit5 bit4
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

  Interrupts
    may be set to trigger when:
      - transmit hold A/B ready
      - receive hold A/B ready
      - receive FIFO A/B full
      - start/end of received break A/B
      - end of coutner/timer count reached
      - change of state on IP0-IP3
    ISR/IMR/IMSR register
      IMSR is (ISR & IMR) -- status only for enabled interrupts
      flags which interrupt sources have/are occuring (ISR), or which events should fire interrupts (IMR)
      bit7 - input port change
      bit6 - delta break B
      bit5 - RxRdy/FFullB
      bit4 - TxRdyB
      bit3 - Counter Ready
      bit2 - delta break A
      bit1 - RxRdy/FFullA
      bit0 - TxRdyA
    IVR register
      defaults to 0x0F on reset, until overwritten (where 0x0F corresponds to "Uninitialized Interrupt" vector)
    Clearing
      ISR[7] cleared by reading IPCR
      ISR[6] cleared by "reset break change interrupt" command on channel B
      ISR[5] cleared by reading RHRB
      ISR[4] cleared by writing to THRB or disabling Tx B
      ISR[3] cleared by writing "stop counter" command (bit is set for each tick in timer mode, or when zero in counter mode)
      ISR[2] cleared by "reset break change interrupt" command on channel A
      IRS[1] cleared by reading RHRA
      ISR[0] cleared by writing to THRA or disabling Tx A
*/
