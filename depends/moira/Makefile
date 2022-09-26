LIBRARY  = libmoira.a
ARFLAGS  = rcs
CPPFLAGS = -std=c++20
OBJECTS  = Moira.o MoiraDebugger.o

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar $(ARFLAGS) $(LIBRARY) $(OBJECTS)

%.o: %.c
	clang $(CPPFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJECTS)

veryclean: clean
	rm -f $(LIBRARY)

remake: veryclean all
