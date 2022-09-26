LIBRARY = libtermbox.a
ARFLAGS = rcs
CFLAGS  = -std=c99
OBJECTS = termbox.o

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar $(ARFLAGS) $(LIBRARY) $(OBJECTS)

%.o: %.c
	clang $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJECTS)

veryclean: clean
	rm -f $(LIBRARY)

remake: veryclean all
