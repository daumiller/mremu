C_COMPILER       = m68k-elf-gcc
ASM_MOT_COMPILER = ../depends/vasm/vasmm68k_mot
ASM_GNU_COMPILER = m68k-elf-as
LINKER           = m68k-elf-ld
LINK_SCRIPT      = bootrom.ld

C_OPTIMIZATION = -O0
C_FLAGS        = -std=c99 -mcpu=68010 -nostdlib -nodefaultlibs \
                 -ffreestanding -nostartfiles -fomit-frame-pointer \
								 -fno-delete-null-pointer-checks -fno-zero-initialized-in-bss \
								 $(C_OPTIMIZATION)
ASM_MOT_FLAGS  = -m68010 -no-opt -no-fpu -Felf
ASM_GNU_FLAGS  = -march=68010 -mcpu=68010 
LINK_FLAGS = -T $(LINK_SCRIPT)

OBJECT_FILES = loader-startup.asm.o \
               loader-main.c.o      \
							 duart.c.o

# ===============================================

all: bootrom

# ===============================================

%.asm.o: %.asm
	$(ASM_MOT_COMPILER) $(ASM_MOT_FLAGS) $^ -o $@

%.c.o: %.c
	$(C_COMPILER) $(C_FLAGS) -c $^ -o $@

%.s.o: %.s
	$(ASM_GNU_COMPILER) $(ASM_GNU_FLAGS) -c $^ -o $@

%.c.s: %.c
	$(C_COMPILER) $(C_FLAGS) -S $^ -o $@

# ===============================================

bootrom: $(OBJECT_FILES)
	$(LINKER) $(LINK_FLAGS) $^ -o $@

# ===============================================

startup-symbols: startup.asm.o
	m68k-elf-objdump --syms startup.asm.o

clean:
	rm -f $(OBJECT_FILES)
	rm -f *.c.s

veryclean: clean
	rm -f bootrom

remake: veryclean all
