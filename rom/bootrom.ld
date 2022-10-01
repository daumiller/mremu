OUTPUT_FORMAT("binary")

SECTIONS
{
  . = 0xE00000;
  startup : { startup.asm.o (.text) }
  kernel  : { kernel.c.o (.text) }
  other   : { * (.text) }
}
