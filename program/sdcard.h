#ifndef SD_CARD_HEADER
#define SD_CARD_HEADER

#include <stdint.h>
#include <stdbool.h>
#include "duart.h"

#define SD_CARD_STATUS_ERROR_LEADING_BIT    0x80
#define SD_CARD_STATUS_ERROR_PARAMETER      0x40
#define SD_CARD_STATUS_ERROR_ADDRESS        0x20
#define SD_CARD_STATUS_ERROR_ERASE_SEQUENCE 0x10
#define SD_CARD_STATUS_ERROR_CRC            0x08
#define SD_CARD_STATUS_ERROR_COMMAND        0x04
#define SD_CARD_STATUS_ERASE_RESET          0x02
#define SD_CARD_STATUS_IDLE_STATE           0x01

#define SD_CARD_STATUS_HAS_ERRORS(x) (x & 0xFC)

typedef struct {
  duart_spi_device spi_device;
  bool             selected;
  bool             spi_initialized;
  bool             card_initialized;
  uint32_t         size_in_mib;
  uint8_t          last_status_byte;
  uint8_t          command_buffer[6];
} SdCard;

bool sdCard_initialize(SdCard* card, duart_spi_device spi_device);
bool sdCard_select    (SdCard* card);
bool sdCard_deselect  (SdCard* card);
bool sdCard_read      (SdCard* card, uint32_t block_number, uint8_t* block_buffer);
bool sdCard_write     (SdCard* card, uint32_t block_number, uint8_t* block_buffer);

void sdCard_writeErrorsToSerial(SdCard* card, duart_serial_port serial_port);

#endif // ifdef SD_CARD_HEADER
