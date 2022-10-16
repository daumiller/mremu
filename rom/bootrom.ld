OUTPUT_FORMAT("binary")

SECTIONS
{
  . = 0xE00000;
  startup : { loader-startup.asm.o (.text) }
  main    : { loader-main.c.o (.text) }
  other   : { * (.text) }
}
