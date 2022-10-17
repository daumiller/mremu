#ifndef DUART_HEADER
#define DUART_HEADER

#include <stdint.h>
#include <stdbool.h>

#define DUART_ADDRESS_BASE 0xF00000

// DUART Register Addresses
//   register addresses are all odd numbers
//     registers are mapped as (base_address + (register_address * 2) + 1)
//     so, the [0x00-0x0F] registers range is mapped to (base_address + [0x01-0x1F])
//   NOTE: Here we're calling "set output port bits" OUTPUT_BITS_CLEAR, and calling "clear output port bits" OUTPUT_BITS_SET,
//         the inverse of the register names. But the DUART inverts the actual output port data when outputting;
//         so using these inverted names is accurate to what actually shows up on the pins:
//           OUTPUT_BITS_CLEAR => set pin values low
//           OUTPUT_BITS_SET   => set pin values high
#define DUART_A_MODE                  ((volatile char*)(DUART_ADDRESS_BASE + (0x0 << 1) + 1))  /* read/write | channel A: mode                  | MR1A/MR2A  */
#define DUART_A_STATUS                ((volatile char*)(DUART_ADDRESS_BASE + (0x1 << 1) + 1))  /* read       | channel A: status                | SRA        */
#define DUART_A_CLOCK_SELECT          ((volatile char*)(DUART_ADDRESS_BASE + (0x1 << 1) + 1))  /*      write | channel A: clock select          | CSRA       */
#define DUART_INTERRUPT_STATUS_MASKED ((volatile char*)(DUART_ADDRESS_BASE + (0x2 << 1) + 1))  /* read       | interrupt status (masked)        | MISR       */
#define DUART_A_COMMAND               ((volatile char*)(DUART_ADDRESS_BASE + (0x2 << 1) + 1))  /*      write | channel A: command               | CRA        */
#define DUART_A_RECEIVE               ((volatile char*)(DUART_ADDRESS_BASE + (0x3 << 1) + 1))  /* read       | channel A: receiver data         | RHRA       */
#define DUART_A_TRANSMIT              ((volatile char*)(DUART_ADDRESS_BASE + (0x3 << 1) + 1))  /*      write | channel A: transmitter data      | THRA       */
#define DUART_INPUT_CHANGE            ((volatile char*)(DUART_ADDRESS_BASE + (0x4 << 1) + 1))  /* read       | input port: changed data         | IPCR       */
#define DUART_AUX_CONTROL             ((volatile char*)(DUART_ADDRESS_BASE + (0x4 << 1) + 1))  /*      write | auxiliary control                | ACR        */
#define DUART_INTERRUPT_STATUS        ((volatile char*)(DUART_ADDRESS_BASE + (0x5 << 1) + 1))  /* read       | interrupt status (not-masked)    | ISR        */
#define DUART_INTERRUPT_MASK          ((volatile char*)(DUART_ADDRESS_BASE + (0x5 << 1) + 1))  /*      write | interrupt mask                   | IMR        */
#define DUART_COUNTER_VALUE_UPPER     ((volatile char*)(DUART_ADDRESS_BASE + (0x6 << 1) + 1))  /* read/write | counter/timer value upper byte   | CTU        */
#define DUART_COUNTER_VALUE_LOWER     ((volatile char*)(DUART_ADDRESS_BASE + (0x7 << 1) + 1))  /* read/write | counter/timer value lower byte   | CTL        */
#define DUART_B_MODE                  ((volatile char*)(DUART_ADDRESS_BASE + (0x8 << 1) + 1))  /* read/write | channel B: mode                  | MR1B/MR2B  */
#define DUART_B_STATUS                ((volatile char*)(DUART_ADDRESS_BASE + (0x9 << 1) + 1))  /* read       | channel B: status                | SRB        */
#define DUART_B_CLOCK_SELECT          ((volatile char*)(DUART_ADDRESS_BASE + (0x9 << 1) + 1))  /*      write | channel B: clock select          | CSRB       */
#define DUART_RESERVED                ((volatile char*)(DUART_ADDRESS_BASE + (0xA << 1) + 1))  /* read       | (reserved/invalid)               |            */
#define DUART_B_COMMAND               ((volatile char*)(DUART_ADDRESS_BASE + (0xA << 1) + 1))  /*      write | channel B: command               | CRB        */
#define DUART_B_RECEIVE               ((volatile char*)(DUART_ADDRESS_BASE + (0xB << 1) + 1))  /* read       | channel B: receiver data         | RHRB       */
#define DUART_B_TRANSMIT              ((volatile char*)(DUART_ADDRESS_BASE + (0xB << 1) + 1))  /*      write | channel B: transmitter data      | THRB       */
#define DUART_INTERRUPT_VECTOR        ((volatile char*)(DUART_ADDRESS_BASE + (0xC << 1) + 1))  /* read/write | interrupt vector                 | IVR        */
#define DUART_INPUT                   ((volatile char*)(DUART_ADDRESS_BASE + (0xD << 1) + 1))  /* read       | input port: values               | IP         */
#define DUART_OUTPUT_CONFIGURATION    ((volatile char*)(DUART_ADDRESS_BASE + (0xD << 1) + 1))  /*      write | output port: configuration       | OPCR       */
#define DUART_COUNTER_START           ((volatile char*)(DUART_ADDRESS_BASE + (0xE << 1) + 1))  /* read       | start counter/timer (by reading) | SCC        */
#define DUART_OUTPUT_BITS_CLEAR       ((volatile char*)(DUART_ADDRESS_BASE + (0xE << 1) + 1))  /*      write | output port: set given bits LOW  | SOPBC      */
#define DUART_COUNTER_STOP            ((volatile char*)(DUART_ADDRESS_BASE + (0xF << 1) + 1))  /* read       | stop counter/timer (by reading)  | STC        */
#define DUART_OUTPUT_BITS_SET         ((volatile char*)(DUART_ADDRESS_BASE + (0xF << 1) + 1))  /*      write | output port: set given bits HIGH | COPBC      */

// DUART Input Pins - SPI
#define DUART_INPUT_CIPO 0x4

// DUART Output Pins - SPI
#define DUART_OUTPUT_SPI_CSA_INVERSE 0x04
#define DUART_OUTPUT_SPI_CSB_INVERSE 0x80
#define DUART_OUTPUT_SPI_CLOCK       0x10
#define DUART_OUTPUT_SPI_COPI        0x40

// DUART Output Pins - LEDs
#define DUART_OUTPUT_LED_RED_INERSE    0x08
#define DUART_OUTPUT_LED_GREEN_INVERSE 0x20

// Inline Input/Output helpers
#define duartOutputHigh(byte) (*DUART_OUTPUT_BITS_SET   = (byte))
#define duartOutputLow(byte)  (*DUART_OUTPUT_BITS_CLEAR = (byte))
#define duartInputRead(mask)  (mask & *DUART_INPUT)

// Inline Serial Ready helpers
#define DUART_STATUS_READY_RECEIVE  0x1
#define DUART_STATUS_READY_TRANSMIT 0x4
#define duartTransferReady(port) (*port & DUART_STATUS_READY_TRANSMIT)
#define duartReceiverReady(port) (*port & DUART_STATUS_READY_RECEIVE)

// ---------------
//     Serial
// ---------------
typedef enum {
  DUART_SERIAL_A,
  DUART_SERIAL_B
} duart_serial_port;

void    duartSerial_writeCharacter(duart_serial_port port, uint8_t character);
void    duartSerial_writeString   (duart_serial_port port, const uint8_t* string);
void    duartSerial_writeBuffer   (duart_serial_port port, const uint8_t* buffer, uint32_t buffer_size);

uint8_t duartSerial_readCharacter(duart_serial_port port);
bool    duartSerial_readLine     (duart_serial_port port, uint8_t* buffer, uint32_t buffer_maximum_size, uint32_t* optional_read_length);
void    duartSerial_readBuffer   (duart_serial_port port, uint8_t* buffer, uint32_t buffer_size);

// ---------------
//     Output
// ---------------
void duartOutput_enable();

// ---------------
//      LEDs
// ---------------
typedef enum {
  DUART_LED_GREEN,
  DUART_LED_RED
} duart_led;

void duartLed_set(duart_led led, bool on);

// ---------------
//      SPI
// ---------------
typedef enum {
  DUART_SPI_A,
  DUART_SPI_B
} duart_spi_device;

void    duartSpi_select(bool spi_a_selected, bool spi_b_selected);
uint8_t duartSpi_transferByte(uint8_t send_byte);
void    duartSpi_readBuffer(uint8_t* receive_buffer, uint32_t length);
void    duartSpi_writeBuffer(uint8_t* send_buffer, uint32_t length);
void    duartSpi_transferBuffer(uint8_t* send_buffer, uint8_t* receive_buffer, uint32_t length);

#endif // ifndef DUART_HEADER
