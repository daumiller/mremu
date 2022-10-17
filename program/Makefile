C_COMPILER       = m68k-elf-gcc
ASM_MOT_COMPILER = ../depends/vasm/vasmm68k_mot
ASM_GNU_COMPILER = m68k-elf-gcc
LINKER           = m68k-elf-ld

C_OPTIMIZATION = -O0
C_FLAGS        = -std=c99 -mcpu=68010 -nostdlib -nodefaultlibs \
                 -ffreestanding -nostartfiles -fomit-frame-pointer \
								 -fno-delete-null-pointer-checks -fno-zero-initialized-in-bss \
								 $(C_OPTIMIZATION) -mpcrel
ASM_MOT_FLAGS  = -m68010 -no-opt -no-fpu -Felf -pic
ASM_GNU_FLAGS  = -march=68010 -mcpu=68010 -fno-zero-initialized-in-bss -fpic -mpcrel
LINK_FLAGS     = -T program.ld

OBJECT_FILES = startup.asm.o \
               main.c.o      \
               duart.c.o     \
               sdcard.c.o    \
               math.S.o

# ===============================================

all: program.bin

# ===============================================

%.asm.o: %.asm
	$(ASM_MOT_COMPILER) $(ASM_MOT_FLAGS) $^ -o $@

%.c.o: %.c
	$(C_COMPILER) $(C_FLAGS) -c $^ -o $@

%.S.o: %.S
	$(ASM_GNU_COMPILER) $(ASM_GNU_FLAGS) -c $^ -o $@

%.c.s: %.c
	$(C_COMPILER) $(C_FLAGS) -S $^ -o $@

# ===============================================

program.bin: $(OBJECT_FILES)
	$(LINKER) $(LINK_FLAGS) $^ -o $@

# ===============================================

symbols: startup.asm.o main.c.o
	m68k-elf-objdump --syms startup.asm.o
	m68k-elf-objdump --syms main.c.o
	m68k-elf-objdump --syms duart.c.o

disassembly: program.bin
	../depends/dis68k/dis68k < $^

# ===============================================

clean:
	rm -f $(OBJECT_FILES)
	rm -f *.c.s

veryclean: clean
	rm -f program.bin

remake: veryclean all
