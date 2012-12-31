CC=clang
CFLAGS=-D_BSD_SOURCE=1 -W -Wall -g -std=c99

all: sem

compiler.c: compiler.y
	bison --defines=tokens.h -o $@ $<

scanner.c: scanner.l tokens.h
	flex --header=scanner.h -o $@ $<

OBJECTS=io.o memory.o compiler.o scanner.o vm.o debugger.o main.o

$(OBJECTS): sem.h version.h

sem: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

debug: sem
	gdb ./sem

valgrind: sem
	valgrind -v --leak-check=full ./sem ../examples/fact.sem

run: sem
	./sem ../examples/fact.sem

clean: 
	rm -f *.o sem

distclean: clean
	rm -f compiler.c tokens.h scanner.c

indent:
	indent -linux compiler.y scanner.l debugger.c vm.c io.c memory.c main.c

.PHONY: all clean distclean run debug valgrind indent
