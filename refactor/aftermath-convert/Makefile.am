ACLOCAL_AMFLAGS=-I m4
AM_CFLAGS=-Wall -Werror

bin_PROGRAMS = aftermath-convert

aftermath_convert_SOURCES = \
	src/main.c \
	src/v16.c \
	src/v16.h \
	src/v16_structs.h


aftermath_convert_CFLAGS = @AFTERMATH_CORE_INCLUDES@ $(AM_CFLAGS)
aftermath_convert_LDADD = @AFTERMATH_CORE_LIBS@
