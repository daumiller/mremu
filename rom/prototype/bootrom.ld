OUTPUT_FORMAT("binary")

SECTIONS
{
  . = 0xE00000;
  rom_asm : { rom.o  (.text) }
  rom_c   : { * (.text) }
}
