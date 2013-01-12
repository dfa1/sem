AM_CFLAGS = -std=c99
AM_YFLAGS = -d
AM_LFLAGS = --header-file=scanner.h

noinst_LIBRARIES = libsem.a
libsem_a_SOURCES=\
	compiler.y 	\
	debugger.c		\
	io.c		\
	memory.c		\
	vm.c		\
	scanner.l 
EXTRA_DIST = sem.h compiler.h scanner.h 
DISTCLEANFILES = scanner.h scanner.c compiler.h compiler.c 
bin_PROGRAMS = sem
sem_SOURCES = main.c 
sem_LDADD = libsem.a


