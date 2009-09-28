#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT(sem, 2.1, davide.angelocola@gmail.com)
AC_CONFIG_SRCDIR([src/debug.c])
AC_CONFIG_HEADER([src/config.h])

dnl Automake init.
AM_INIT_AUTOMAKE([foreign no-define 1.9 -Wall])

# Checks for programs.
AC_PROG_CC
gl_EARLY
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# gnulib
gl_INIT

# Output.
AC_CONFIG_FILES([
doc/Makefile
examples/Makefile
misc/Makefile
gnulib/Makefile
src/Makefile
Makefile
])

AC_OUTPUT
