AM_CFLAGS=-std=c99
lib_LIBRARIES = libsem.a
libsem_a_SOURCES=\
	compiler.y 	\
	debugger.c		\
	io.c		\
	memory.c		\
	vm.c		\
	scanner.l		

bin_PROGRAMS = sem

sem_SOURCES = \
	main.c		
sem_LDADD=libsem.a
EXTRA_DIST = \
	version.h \
	sem.h 		\
	compile.h	

AM_YFLAGS = -d

