#include "duart.h"
#include "sdcard.h"

// https://www.lddgo.net/en/encrypt/crc

static void hexString8(uint8_t hex, uint8_t* buffer) {
  uint8_t nibble = (hex & 0xF0) >> 4;
  if(nibble < 10) {
    buffer[0] = '0' + nibble;
  } else {
    buffer[0] = 'A' + nibble - 10;
  }
  nibble = (hex & 0xF);
  if(nibble < 10) {
    buffer[1] = '0' + nibble;
  } else {
    buffer[1] = 'A' + nibble - 10;
  }
}

static uint8_t sdCardSpi_readByte() {
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

static void sdCardSpi_readBytes(uint8_t* buffer, uint8_t length) {
  if(length == 0) { return; }
  buffer[0] = sdCardSpi_readByte();
  ++buffer;
  --length;
  while(length) {
    buffer[0] = duartSpi_transferByte(0xFF);
    ++buffer;
    --length;
  }
}

bool sdCard_initialize(SdCard* card, duart_spi_device spi_device) {
  card->spi_device              = spi_device;
  card->ready_for_commands      = false;
  card->initialization_complete = false;
  card->high_capacity           = false;
  card->last_status_byte        = 0x00;

  // setup COPI & CLOCK
  duartOutputHigh(DUART_OUTPUT_SPI_COPI); // set, and keep, COPI high, whenever not transferring
  duartOutputLow(DUART_OUTPUT_SPI_CLOCK); // set, and keep, CLOCK low, whenever not transferring
  // wait for COPI+CLOCK to stabilize
  for(uint8_t stabilize=0; stabilize<128; ++stabilize) {}
  
  // select device, and wait for CIPO to stabilize
  duartSpi_select(card->spi_device == DUART_SPI_A, card->spi_device == DUART_SPI_B);
  for(uint8_t stabilize=0; stabilize<8; ++stabilize) {
    duartOutputHigh(DUART_OUTPUT_SPI_CLOCK);
    stabilize &= stabilize;
    duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
  }

  // check if SD SPI mode already enabled (if so, CIPO will stay high while we read a few clocks)
  if(duartSpi_transferByte(0xFF) == 0xFF) {
    card->ready_for_commands = true;
    card->last_status_byte   = SD_CARD_STATUS_IDLE_STATE;
  }
  duartSpi_select(false, false);

  if(!card->ready_for_commands) {
    // enable SD SPI mode
    // 1) with /CS high (not-selected), clock out >74 cycles
    // 2) select device, send command zero, return COPI high
    // 3) send clocks until CIPO goes high (or errors out)
    // 4) read back status (involves some extra clocks, waiting for the first CIPO low transition)

    // 1) with /CS high (not-selected), clock out >74 cycles
    duartSpi_select(false, false);
    for(uint8_t idx=0; idx<128; ++idx) {
      duartOutputHigh(DUART_OUTPUT_SPI_CLOCK);
      for(uint8_t delay=0; delay<1; ++delay) {}
      duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
    }

    // 2) select device, send command zero, return COPI high
    duartSpi_select(card->spi_device == DUART_SPI_A, card->spi_device == DUART_SPI_B);
    uint8_t command_zero[] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
    duartSpi_writeBuffer(command_zero, 6);
    duartOutputHigh(DUART_OUTPUT_SPI_COPI);

    // 3) send clocks until CIPO goes high (or errors out)
    uint8_t attempts = 0;
    while(duartInputRead(DUART_INPUT_CIPO) == 0x00) {
      duartOutputHigh(DUART_OUTPUT_SPI_CLOCK);
      card->last_status_byte &= card->last_status_byte;
      duartOutputLow(DUART_OUTPUT_SPI_CLOCK);
      ++attempts;
      if(attempts == 255) {
        duartSpi_select(false, false);
        return false;
      }
    }

    // 4) read back status (involves some extra clocks, waiting for the first CIPO low transition)
    card->last_status_byte = sdCardSpi_readByte();
  }
  card->ready_for_commands = true;

  // make sure device selected, and give it a stabilization delay
  duartSpi_select(card->spi_device == DUART_SPI_A, card->spi_device == DUART_SPI_B);
  for(uint8_t delay=0; delay<8; ++delay) {}

  // post-ready-for-commands initialization
  uint8_t command_eight[]       = { 0x48, 0x00, 0x00, 0x01, 0xAA, 0x87 };
  uint8_t command_fifty_eight[] = { 0x7A, 0x00, 0x00, 0x00, 0x00, 0xFD };
  uint8_t command_one[]         = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xF9 };
  uint8_t five_byte_response[]  = { 0x00, 0x00, 0x00, 0x00, 0x00 };
  uint8_t hex_buffer[] = { 0x00, 0x00, 0x00 };

  // send command 8, read response
  duartSpi_writeBuffer(command_eight, 6);
  duartOutputHigh(DUART_OUTPUT_SPI_COPI);
  sdCardSpi_readBytes(five_byte_response, 5);
  card->last_status_byte = five_byte_response[0];

  // if command 8 fails, use command 58 instead
  if(card->last_status_byte > SD_CARD_STATUS_IDLE_STATE) {
    duartSpi_writeBuffer(command_fifty_eight, 6);
    duartOutputHigh(DUART_OUTPUT_SPI_COPI);
    sdCardSpi_readBytes(five_byte_response, 5);
    card->last_status_byte = five_byte_response[0];
  }
  // if that fails... we've lost
  if(card->last_status_byte > SD_CARD_STATUS_IDLE_STATE) {
    duartSpi_select(false, false);
    return false;
  }

  // wait for card init
  uint8_t init_attempts = 0;
  while(true) {
    uint8_t command_fifty_five[] = { 0x77, 0x00, 0x00, 0x00, 0x00, 0x65 };
    duartSpi_writeBuffer(command_fifty_five, 6);
    duartOutputHigh(DUART_OUTPUT_SPI_COPI);
    card->last_status_byte = sdCardSpi_readByte();
    if(card->last_status_byte > SD_CARD_STATUS_IDLE_STATE) {
      duartSpi_select(false, false);
      return false;
    }

    uint8_t command_forty_one[]  = { 0x69, 0x40, 0x00, 0x00, 0x00, 0x77 };
    duartSpi_writeBuffer(command_forty_one, 6);
    duartOutputHigh(DUART_OUTPUT_SPI_COPI);
    card->last_status_byte = sdCardSpi_readByte();
    if(card->last_status_byte == 0x00) { break; }

    if(init_attempts == 255) {
      duartSpi_select(false, false);
      return false;
    }
    ++init_attempts;
  }

  // set block size to 512 bytes
  uint8_t command_sixteen_512[] = { 0x50, 0x00, 0x00, 0x02, 0x00, 0x15 };
  duartSpi_writeBuffer(command_sixteen_512, 6);
  duartOutputHigh(DUART_OUTPUT_SPI_COPI);
  card->last_status_byte = sdCardSpi_readByte();
  if(card->last_status_byte == 0x00) {
    card->initialization_complete = true;
  } else {
    duartSpi_select(false, false);
    return false;
  }

  // deselect device
  duartSpi_select(false, false);

  // report back status
  return (card->ready_for_commands && card->initialization_complete);
}

void sdCard_writeErrorsToSerial(SdCard* card, duart_serial_port serial_port) {
  if(card->ready_for_commands == false) {
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
