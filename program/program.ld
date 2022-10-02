OUTPUT_FORMAT("binary")

SECTIONS
{
  startup : { startup.asm.o (.text) }
  main    : { main.c.o (.text) }
  other   : { * (.text) }
}
