CC=clang
CFLAGS=-D_BSD_SOURCE=1 -W -Wall -Wno-unused-parameter -g -std=c99

all: sem

compiler.c: compiler.y
	bison --defines=tokens.h -o $@ $<

scanner.c: scanner.l tokens.h
	flex --header=scanner.h -o $@ $<

OBJECTS=io.o memory.o compiler.o scanner.o vm.o debugger.o main.o

$(OBJECTS): sem.h version.h

sem: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

test: test_memory
	
test_memory: test_memory.c
	$(CC) $(CFLAGS) test_memory.c memory.c -o $@
	./test_memory

clean: 
	rm -f *.o sem test_memory

distclean: clean
	rm -f compiler.c tokens.h scanner.c

indent:
	indent -linux sem.h version.h compiler.y scanner.l debugger.c vm.c io.c memory.c main.c test_*.c
	rm -f *~ 

.PHONY: all clean distclean run debug indent test
