ASSEMBLER = depends/vasm/vasmm68k_mot
ASM_FLAGS = -m68010 -no-opt -no-fpu -Fbin
ASM_O_FLAGS = -m68010 -no-opt -no-fpu -Felf

COMPILER  = clang
CPP_FLAGS = -std=c++20 -I./depends
CPP_LIBS  = -L./depends/moira -lmoira -lstdc++ -L./depends/termbox2 -ltermbox -L./depends/vterm -lvterm
CPP_OBJS  = machine/rosco_m68k.o           \
            machine/interrupt_controller.o \
            machine/duart_68681.o          \
            machine/duart_68681_uart.o     \
            interface/disassembly.o        \
            interface/registers.o          \
            interface/memory.o             \
            interface/terminal.o           \
            interface/button.o             \
            interface/helpers.o            \
            main.o
CPP_DEBUG_OBJS = $(CPP_OBJS:.o=.debug.o)

M68K_COMPILER = m68k-elf-gcc
M68K_C_FLAGS  = -std=c99 -nostdlib -nodefaultlibs -ffreestanding -nostartfiles -mcpu=68010 -fomit-frame-pointer -fno-delete-null-pointer-checks -fno-zero-initialized-in-bss -O0
M68K_LINKER   = m68k-elf-ld
M68K_L_FLAGS  = -T bootrom.ld

# ===============================================

all: mremu

release: mremu

debug: mremu-debug

# ===============================================

boot.o: boot.c
	$(M68K_COMPILER) $(M68K_C_FLAGS) -c $^ -o $@

duart.o: duart.c
	$(M68K_COMPILER) $(M68K_C_FLAGS) -c $^ -o $@

boot.s: boot.c
	$(M68K_COMPILER) $(M68K_C_FLAGS) -S $^ -o $@

bootrom: rom.o boot.o duart.o
	$(M68K_LINKER) $(M68K_L_FLAGS) $^ -o $@

rom: rom.bin

rom.o: rom.asm
	$(ASSEMBLER) $(ASM_O_FLAGS) $^ -o $@

rom.bin: rom.asm
	$(ASSEMBLER) $(ASM_FLAGS) $^ -o $@

# ===============================================

mremu: $(CPP_OBJS)
	$(COMPILER) $(CPP_LIBS) $^ -o $@

mremu-debug: $(CPP_DEBUG_OBJS)
	$(COMPILER) $(CPP_LIBS) $^ -o $@

%.debug.o: %.cpp
	$(COMPILER) $(CPP_FLAGS) --debug -c $^ -o $@

%.o: %.cpp
	$(COMPILER) $(CPP_FLAGS) -c $^ -o $@

# ===============================================
clean:
	rm -f $(CPP_OBJS)
	rm -f $(CPP_DEBUG_OBJS)

veryclean: clean
	rm -f *.bin
	rm -f mremu

remake: veryclean all
