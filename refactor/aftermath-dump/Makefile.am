ACLOCAL_AMFLAGS=-I m4
AM_CFLAGS=-Wall -Werror

bin_PROGRAMS = aftermath-dump

aftermath_dump_SOURCES = src/main.c

aftermath_dump_CFLAGS = @AFTERMATH_CORE_INCLUDES@ $(AM_CFLAGS)
aftermath_dump_LDADD = @AFTERMATH_CORE_LIBS@

