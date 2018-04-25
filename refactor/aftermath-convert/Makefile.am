ACLOCAL_AMFLAGS=-I m4
AM_CFLAGS=-Wall -Werror

bin_PROGRAMS = aftermath-convert

aftermath_convert_SOURCES = src/main.c

aftermath_convert_CFLAGS = @AFTERMATH_CORE_INCLUDES@ $(AM_CFLAGS)
aftermath_convert_LDADD = @AFTERMATH_CORE_LIBS@
