COMPILER = clang
CFLAGS   = -I./ -I./src -std=c99 -c
OBJECTS  = src/vterm.o    \
           src/encoding.o \
           src/keyboard.o \
           src/mouse.o    \
           src/parser.o   \
           src/pen.o      \
           src/screen.o   \
           src/state.o    \
           src/unicode.o
DEBUG_OBJS = $(OBJECTS:.o=.debug.o)

all: libvterm.a

debug: libvterm-debug.a

libvterm-debug.a: $(DEBUG_OBJS)
	ar rcs $@ $^

%.debug.o: %.c
	$(COMPILER) $(CFLAGS) --debug $^ -o $@

libvterm.a: $(OBJECTS)
	ar rcs $@ $^

%.o: %.c
	$(COMPILER) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(DEBUG_OBJS)

veryclean: clean
	rm -f libvterm.a
	rm -f libvterm-debug.a
