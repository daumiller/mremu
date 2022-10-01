#include <stdint.h>
#include <stdbool.h>

void duartWriteByte(uint8_t byte);
void duartWriteString(const char* string);
void duartWriteLine(const char* line);
uint8_t duartReadByte(bool echo);
void duartReadLine(char* buffer, uint16_t max_length, uint16_t* read_length, bool echo);
