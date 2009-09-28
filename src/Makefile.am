bin_PROGRAMS = sem

sem_SOURCES = \
	compile.y 	\
	debug.c		\
	eval.c		\
	main.c		\
	scan.l		

sem_CPPFLAGS = \
        -I $(top_srcdir)/gnulib   

sem_LDADD = \
        $(top_builddir)/gnulib/libgnu.la 

EXTRA_DIST = \
	sem.h 		\
	compile.h	

AM_YFLAGS = -d

