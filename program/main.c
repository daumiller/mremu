#include <stdint.h>
#include <stdbool.h>
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
}

static void hexString8(uint8_t byte, uint8_t* buffer) {
    uint8_t nibble;
    nibble = (byte & 0xF0) >> 4; if(nibble < 10) { buffer[0] = '0' + nibble; } else { buffer[0] = 'A' + nibble - 10; }
    nibble = (byte & 0x0F);      if(nibble < 10) { buffer[1] = '0' + nibble; } else { buffer[1] = 'A' + nibble - 10; }
    buffer[2] = 0x00;
}

static void decString(uint32_t dec, uint8_t* buffer) {
  uint8_t dec_buffer[16];
  uint8_t digit_number = 0;
  uint8_t dec_buffer_location = 15;
  while(true) {
    uint32_t digit = dec % 10;
    dec -= digit;
    dec /= 10;
    dec_buffer[dec_buffer_location] = (uint8_t)(digit + '0');
    if(dec == 0) { break; }
    dec_buffer_location--;
    digit_number++;
    if(digit_number %3 == 0) {
      dec_buffer[dec_buffer_location] = ',';
      dec_buffer_location--;
    }
  }

  for(uint8_t idx=dec_buffer_location; idx<16; ++idx) {
    buffer[0] = dec_buffer[idx];
    buffer++;
  }
  buffer[0] = 0x00;
}

uint32_t endianSwap32(uint32_t value) {
  uint32_t result = 0;
  result |= value & 0xFF; result <<= 8; value >>= 8;
  result |= value & 0xFF; result <<= 8; value >>= 8;
  result |= value & 0xFF; result <<= 8; value >>= 8;
  result |= value & 0xFF;
  return result;
}

uint16_t endianSwap16(uint16_t value) {
  uint16_t result = ((value & 0xFF00) >> 8) | (value & 0xFF);
}

typedef struct {
  uint8_t status;   // 0x80: bootable, 0x00: inactive, else: invalid
  uint8_t address_first_head;
  uint8_t address_first_sector; // actually lower6 bits are sector, upper2 bits are MSBs of cylinder
  uint8_t address_first_cylinder;
  uint8_t type; // 00:empty, 06:fat16b:chs, 07:NTFS, 0b:fat32:CHS, 0C:fat32:LBA, 0e:fat16b:lba, 82:linux-swap, 83:any-linux, 85:ext, 96:iso9660, AF:hfs(+), 
  uint8_t address_last_head;
  uint8_t address_last_sector;
  uint8_t address_last_cylinder;
  uint32_t address_first;
  uint32_t sector_count;
} PartitionTableEntry;

// 00 | FE FF FF | 0B | FE FF FF | 00 08 00 00 | 00 A8 73 00 | status:not-bootable, CHS:invalid, type:fat32, CHS:invalid, start:524,288, sectors:11,039,488
// 00 | FE FF FF | 0B | FE FF FF | 00 B8 73 00 | 00 A8 73 00 |
// 00 | FE FF FF | 0B | FE FF FF | 00 68 E7 00 | 00 A8 73 00 |
// 00 | FE FF FF | 05 | FE FF FF | 00 10 5B 01 | 00 90 73 00 |
// 00B87300

void main(uint32_t startup_location) {
  duartOutput_enable();

  uint8_t hex24[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  hexString24(startup_location, hex24);
  duartSerial_writeString(DUART_SERIAL_A, "Relocatable code running from 0x");
  duartSerial_writeString(DUART_SERIAL_A, hex24);
  duartSerial_writeString(DUART_SERIAL_A, "\r\n");

  duartSerial_writeString(DUART_SERIAL_A, "Looking for SD card... ");
  SdCard sdcard;
  bool has_sdcard = sdCard_initialize(&sdcard, DUART_SPI_B);
  if(has_sdcard) {
    duartSerial_writeString(DUART_SERIAL_A, "Found!\r\n");
    duartSerial_writeString(DUART_SERIAL_A, "  SD Card Capacity: ");
    uint8_t decimal_buffer[16];
    decString(sdcard.size_in_mib, decimal_buffer);
    duartSerial_writeString(DUART_SERIAL_A, decimal_buffer);
    duartSerial_writeString(DUART_SERIAL_A, " MiB\r\n");

    uint8_t mbr[512];
    if(!sdCard_read(&sdcard, 0, mbr)) {
      duartSerial_writeString(DUART_SERIAL_A, "Error reading MBR :( ...\r\n");
    } else {
      // Partition Table Entries (16 bytes each): 0x01BE, 0x01CE, 0x01DE, 0x01EE
      for(int idx=0; idx<4; ++idx) {
        PartitionTableEntry* entry = (PartitionTableEntry*)(mbr+0x1BE + (idx<<4));
        duartSerial_writeString(DUART_SERIAL_A, "Partition -- Status: ");
        hexString8(entry->status, hex24); duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, ", Type: ");
        hexString8(entry->type, hex24); duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, ", Starting Address: ");
        entry->address_first = endianSwap32(entry->address_first);
        decString(entry->address_first, decimal_buffer); duartSerial_writeString(DUART_SERIAL_A, decimal_buffer);
        duartSerial_writeString(DUART_SERIAL_A, ", Sector Count: ");
        entry->sector_count = endianSwap32(entry->sector_count);
        decString(entry->sector_count, decimal_buffer); duartSerial_writeString(DUART_SERIAL_A, decimal_buffer);
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");
      }
      
      // FAT32
      if(((PartitionTableEntry*)(mbr+0x1BE))->type == 0x0B) {
        duartSerial_writeString(DUART_SERIAL_A, "FAT32 Filesystem on Parition 0\r\n");
        uint32_t boot_sector = ((PartitionTableEntry*)(mbr+0x1BE))->address_first;
        sdCard_read(&sdcard, boot_sector, mbr);

        duartSerial_writeString(DUART_SERIAL_A, "  OEM Name: ");
        for(uint16_t idx=0; idx<8; ++idx) {
          duartSerial_writeCharacter(DUART_SERIAL_A, mbr[0x03+idx]);
        }
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");

        duartSerial_writeString(DUART_SERIAL_A, "  Bytes per Sector: ");
        uint16_t bytes_per_sector = ((uint16_t)mbr[0x0B]) | (((uint16_t)mbr[0x0C]) << 8);
        decString((uint32_t)bytes_per_sector, hex24);
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");

        duartSerial_writeString(DUART_SERIAL_A, "  Sectors per Cluster: ");
        uint8_t sectors_per_cluster = *(mbr + 0x0D);
        decString((uint32_t)sectors_per_cluster, hex24);
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");

        duartSerial_writeString(DUART_SERIAL_A, "  Reserved Sectors: ");
        uint16_t sectors_reserved = ((uint16_t)mbr[0x0E]) | (((uint16_t)mbr[0x0F]) << 8);
        sectors_reserved = endianSwap16(sectors_reserved);
        decString((uint32_t)sectors_reserved, hex24);
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");

        duartSerial_writeString(DUART_SERIAL_A, "  Number of Sectors: ");
        uint32_t sector_cont = *((uint32_t*)(mbr + 0x20));
        sector_cont = endianSwap32(sector_cont);
        decString(sector_cont, hex24);
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");

        duartSerial_writeString(DUART_SERIAL_A, "  Root Directory Cluster: ");
        uint32_t root_cluster = *((uint32_t*)(mbr + 0x2C));
        root_cluster = endianSwap32(root_cluster);
        decString(root_cluster, hex24);
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");

        duartSerial_writeString(DUART_SERIAL_A, "  Volume Name: ");
        for(uint16_t idx=0; idx<11; ++idx) {
          duartSerial_writeCharacter(DUART_SERIAL_A, mbr[0x47+idx]);
        }
        duartSerial_writeString(DUART_SERIAL_A, "\r\n");
      }

      // dump sector 8
      sdCard_read(&sdcard, 8, mbr);
      duartSerial_writeString(DUART_SERIAL_A, "Sector 8 Dump\r\n");
      for(uint16_t idx=0; idx<512; ++idx) {
        hexString8(mbr[idx], hex24);
        hex24[2] = ' ';
        hex24[3] = 0x00;
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        if((idx & 0x1F) == 0x1F) { duartSerial_writeString(DUART_SERIAL_A, "\r\n"); }
      }
      // write sector 8
      mbr[  0]='H'; mbr[  1]='e'; mbr[  2]='l'; mbr[  3]='l'; mbr[  4]='o'; mbr[  5]=0x00;
      mbr[506]='W'; mbr[507]='o'; mbr[508]='r'; mbr[509]='l'; mbr[510]='d'; mbr[511]=0x00;
      if(sdCard_write(&sdcard, 8, mbr)) {
        duartSerial_writeString(DUART_SERIAL_A, "Writing Succeeded!\r\n");
      } else {
        duartSerial_writeString(DUART_SERIAL_A, "Writing Failed...\r\n");
      }
      // dump sector 8
      sdCard_read(&sdcard, 8, mbr);
      duartSerial_writeString(DUART_SERIAL_A, "Sector 8 Dump\r\n");
      for(uint16_t idx=0; idx<512; ++idx) {
        hexString8(mbr[idx], hex24);
        hex24[2] = ' ';
        hex24[3] = 0x00;
        duartSerial_writeString(DUART_SERIAL_A, hex24);
        if((idx & 0x1F) == 0x1F) { duartSerial_writeString(DUART_SERIAL_A, "\r\n"); }
      }
    }
  } else {
    duartSerial_writeString(DUART_SERIAL_A, "not found\r\n");
  }

  bool on_off = true;
  while(true) {
    if(on_off) {
      duartLed_set(DUART_LED_GREEN, true);
      duartLed_set(DUART_LED_RED, false);
    } else {
      duartLed_set(DUART_LED_GREEN, false);
      duartLed_set(DUART_LED_RED, true);
    }
    for(uint16_t idx=0; idx<65534; ++idx) {}
    on_off = !on_off;
  }
}
