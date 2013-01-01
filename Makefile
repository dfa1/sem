PREFIX=/usr/local
CC=clang
CFLAGS=-W -Wall -Wno-unused-parameter -g -std=c99

all: build/sem test 

build:
	mkdir build

src/scanner.c: src/scanner.l src/tokens.h
	flex --header=src/scanner.h -o $@ $<

src/compiler.c: src/compiler.y
	bison -o $@ $<

OBJECTS=build/io.o build/memory.o build/scanner.o build/compiler.o build/vm.o build/debugger.o build/main.o

$(OBJECTS): build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/sem: build $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

test: build build/test_memory
	
build/test_memory: src/test_memory.c src/memory.c
	$(CC) $(CFLAGS) src/test_memory.c src/memory.c -o $@
	./build/test_memory

clean: 
	rm -fr build 

distclean: clean
	rm -f src/compiler.c src/scanner.c src/scanner.h

install: build/sem
	install build/sem ${PREFIX}/bin

indent:
	cd src && indent -linux tokens.h sem.h version.h compiler.y scanner.l debugger.c vm.c io.c memory.c main.c test_*.c

.PHONY: all clean distclean run debug indent test
