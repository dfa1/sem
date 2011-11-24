CFLAGS=-W -Wall -g -std=c99

all: sem

compile.c: compile.y
	bison --defines=tokens.h -o $@ $<

scan.c: scan.l tokens.h
	flex -o $@ $<

OBJECTS=compile.o scan.o eval.o main.o

$(OBJECTS): sem.h version.h

# TODO: debugger as separate binary
sem: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

debug: sem
	gdb sem

run: sem
	./sem ../examples/fact.sem

clean: 
	rm -f *.o sem

distclean: clean
	rm -f compile.c tokens.h scan.c


.PHONY: all clean distclean
