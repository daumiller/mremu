ASSEMBLER = depends/vasm/vasmm68k_mot
ASM_FLAGS = -m68010 -no-opt -no-fpu -Fbin

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

# ===============================================

all: mremu

release: mremu

debug: mremu-debug

# ===============================================

rom: rom.bin

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
