#include "duart.h"
#include "sdcard.h"

static void hexString24(uint32_t hex, uint8_t* buffer) {
  for(uint8_t idx=0; idx<6; ++idx) {
    uint8_t nibble = (hex & 0xF00000) >> 20;
    if(nibble < 10) {
      buffer[0] = '0' + nibble;
    } else {
      buffer[0] = 'A' + nibble - 10;
    }
    ++buffer;
    hex <<= 4;
  }
  buffer[0] = 0x00;
}

static void hexString8(uint8_t byte, uint8_t* buffer) {
    uint8_t nibble;
    nibble = (byte & 0xF0) >> 4; if(nibble < 10) { buffer[0] = '0' + nibble; } else { buffer[0] = 'A' + nibble - 10; }
    nibble = (byte & 0x0F);      if(nibble < 10) { buffer[1] = '0' + nibble; } else { buffer[1] = 'A' + nibble - 10; }
    buffer[2] = 0x00;
}

// CRCs: https://www.lddgo.net/en/encrypt/crc

// reading the first byte of a command result requires a specialized function.
// SPI transfer doesn't actually start until CIPO goes low,
// which may be a variable number of clocks cycles after a command is sent.
static uint8_t sdCardSpi_readLeadingResponseByte() {
  uint8_t read_byte = 0x00;
  bool   first_zero = false;
  uint8_t attempts  = 0;

  // keep COPI high during reads
  duartOutputHigh(DUART_OUTPUT_SPI_COPI);

  // using a 1-based index, so we can decrement the value within the loop
  for(uint8_t index=1; index<9; ++index) {
    // break if no first-zero within (some arbitrary number of) clocks
    if(attempts == 128) { return 0xFF; }

    // shift read_byte value; doesn't matter until after first bit read
    read_byte <<= 1;

    // set clock high, and wait a bit (even out duty cycle)
    duartOutputHigh(DUART_OUTPUT_SPI_CLOCK);
    for(uint8_t delay=0; delay<2; ++delay) {}

    // read input, discarding anything before first zero bit
    if(duartInputRead(DUART_INPUT_CIPO)) {
      if(first_zero) {
        read_byte |= 1;
      } else {
        --index;
      }
    } else {
      first_zero = true;
    }
    
    // set clock low
    duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
    if(!first_zero) { ++attempts; }
  }

  return read_byte;
}

// send a six byte command; read the first (status) response byte
static void sdCardSpi_command(SdCard* card, uint8_t command, uint32_t argument, uint8_t crc) {
  card->command_buffer[5] = crc;
  card->command_buffer[4] = argument & 0xFF; argument >>= 8;
  card->command_buffer[3] = argument & 0xFF; argument >>= 8;
  card->command_buffer[2] = argument & 0xFF; argument >>= 8;
  card->command_buffer[1] = argument & 0xFF;
  card->command_buffer[0] = 0x40 + command;

  duartSpi_writeBuffer(card->command_buffer, 6);
  duartOutputHigh(DUART_OUTPUT_SPI_COPI); // keep COPI high while not transferring
  card->last_status_byte = sdCardSpi_readLeadingResponseByte();
}

static inline bool sdCardSpi_waitForStartToken() {
  for(uint8_t attempts=0; attempts<255; ++attempts) {
    if(duartSpi_transferByte(0xFF) == 0xFE) { return true; }
  }
  return false;
}

// select device, and wait for CIPO to stabilize (high)
bool sdCard_select(SdCard* card) {
  duartSpi_select(card->spi_device == DUART_SPI_A, card->spi_device == DUART_SPI_B);
  for(uint8_t count=0; count<8; ++count) {
    if(duartSpi_transferByte(0xFF) == 0xFF) {
      card->selected = true;
      return true;
    }
  }
  return false;
}

// deselect device, and wait for CIPO to stabilize (low)
bool sdCard_deselect(SdCard* card) {
  duartSpi_select(false, false);
  for(uint8_t count=0; count<8; ++count) {
    if(duartSpi_transferByte(0xFF) == 0x00) {
      card->selected = false;
      return true;
    }
  }
  return false;
}

// dump errors to serial
void sdCard_writeErrorsToSerial(SdCard* card, duart_serial_port serial_port) {
  if(card->spi_initialized == false) {
    duartSerial_writeString(serial_port, "SD Error : card not found, or failed initialization\r\n");
    return;
  }
  if(card->last_status_byte & SD_CARD_STATUS_ERROR_LEADING_BIT   ) { duartSerial_writeString(serial_port, "SD Error : wrong leading bit   \r\n"); }
  if(card->last_status_byte & SD_CARD_STATUS_ERROR_PARAMETER     ) { duartSerial_writeString(serial_port, "SD Error : parameter error     \r\n"); }
  if(card->last_status_byte & SD_CARD_STATUS_ERROR_ADDRESS       ) { duartSerial_writeString(serial_port, "SD Error : address error       \r\n"); }
  if(card->last_status_byte & SD_CARD_STATUS_ERROR_ERASE_SEQUENCE) { duartSerial_writeString(serial_port, "SD Error : erase sequence error\r\n"); }
  if(card->last_status_byte & SD_CARD_STATUS_ERROR_CRC           ) { duartSerial_writeString(serial_port, "SD Error : CRC error           \r\n"); }
  if(card->last_status_byte & SD_CARD_STATUS_ERROR_COMMAND       ) { duartSerial_writeString(serial_port, "SD Error : illegal command     \r\n"); }
}

bool sdCard_initialize(SdCard* card, duart_spi_device spi_device) {
  card->spi_device       = spi_device;
  card->spi_initialized  = false;
  card->card_initialized = false;
  card->last_status_byte = 0x00;
  card->size_in_mib      = 0;

  // set COPI & CLOCK, wait to stabilize
  duartOutputHigh(DUART_OUTPUT_SPI_COPI); // keep COPI high whenever not transferring
  duartOutputLow(DUART_OUTPUT_SPI_CLOCK); // keep, CLOCK low whenever not transferring
  for(uint8_t stabilize=0; stabilize<128; ++stabilize) {}
  
  // select card, this will return false if not yet spi_initialized
  if(sdCard_select(card)) {
    card->spi_initialized  = true;
    card->last_status_byte = SD_CARD_STATUS_IDLE_STATE;
  }

  // initialize SPI if needed (cold boot)
  if(!card->spi_initialized) {
    // 1) with /CS high (not-selected), clock out >74 cycles
    sdCard_deselect(card);
    for(uint8_t idx=0; idx<128; ++idx) {
      duartOutputHigh(DUART_OUTPUT_SPI_CLOCK);
      for(uint8_t delay=0; delay<1; ++delay) {}
      duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
    }

    // 2) select device, send command zero
    sdCard_select(card); // this will return false, we'll get CIPO setup next, after command zero
    sdCardSpi_command(card, 0, 0, 0x95);

    // 3) send clocks until CIPO goes high (or errors out)
    uint8_t attempts = 0;
    while(duartInputRead(DUART_INPUT_CIPO) == 0x00) {
      duartOutputHigh(DUART_OUTPUT_SPI_CLOCK);
      card->last_status_byte &= card->last_status_byte;
      duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
      ++attempts;
      if(attempts == 255) {
        sdCard_deselect(card);
        return false;
      }
    }

    // 4) read back status (involves some extra clocks, waiting for the first CIPO low transition)
    card->last_status_byte = sdCardSpi_readLeadingResponseByte();
    card->spi_initialized = true;
  }

  // send command 8, read response
  sdCardSpi_command(card, 8, 0x01AA, 0x87);
  duartSpi_readBuffer(card->command_buffer, 4);
  // if command 8 fails, use command 58 instead
  if(card->last_status_byte > SD_CARD_STATUS_IDLE_STATE) {
    sdCardSpi_command(card, 58, 0, 0xFD);
    duartSpi_readBuffer(card->command_buffer, 4);
  }
  // if that fails... we've lost
  if(card->last_status_byte > SD_CARD_STATUS_IDLE_STATE) {
    sdCard_deselect(card);
    return false;
  }

  // wait for card to initialize itself
  uint8_t init_attempts = 0;
  while(true) {
    sdCardSpi_command(card, 55, 0, 0x65);
    if(card->last_status_byte > SD_CARD_STATUS_IDLE_STATE) {
      sdCard_deselect(card);
      return false;
    }

    sdCardSpi_command(card, 41, 0x40000000, 0x77);
    if(card->last_status_byte == 0x00) { break; }

    if(init_attempts == 255) {
      sdCard_deselect(card);
      return false;
    }
    ++init_attempts;
  }

  // set block size to 512 bytes
  sdCardSpi_command(card, 16, 512, 0x15);
  if(card->last_status_byte > 0x00) {
    sdCard_deselect(card);
    return false;
  }

  // get card size (via reading CSD)
  sdCardSpi_command(card, 9, 0, 0xAF);
  if(card->last_status_byte > 0x00) {
    sdCard_deselect(card);
    return false;
  }
  // wait for starting token of CSD response
  if(!sdCardSpi_waitForStartToken()) {
    sdCard_deselect(card);
    return false;
  }
  // read CSD response (16 bytes data + 2 bytes CRC)
  uint8_t buffer[18];
  duartSpi_readBuffer(buffer, 18);
  if(buffer[0] != 0x40) {
    sdCard_deselect(card);
    return false;
  }
  card->size_in_mib = 0;
  card->size_in_mib |= buffer[7]; card->size_in_mib <<= 8;
  card->size_in_mib |= buffer[8]; card->size_in_mib <<= 8;
  card->size_in_mib |= buffer[9];
  card->size_in_mib >>= 1;

  // success!
  card->card_initialized = true;
  return true;
}
