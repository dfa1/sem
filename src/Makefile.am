AM_CFLAGS = -std=c99
AM_LFLAGS = --header-file=scanner.h

noinst_LIBRARIES = libsem.a
libsem_a_SOURCES = \
	scanner.l  \
	compiler.y \
	debugger.c \
	io.c       \
	memory.c   \
	vm.c

EXTRA_DIST = sem.h tokens.h 
DISTCLEANFILES = scanner.h scanner.c compiler.c 

bin_PROGRAMS = sem
sem_SOURCES = main.c 
sem_LDADD = libsem.a

noinst_PROGRAMS = test_memory
test_memory_SOURCES = test_memory.c 
test_memory_LDADD = libsem.a

TESTS = test_memory
