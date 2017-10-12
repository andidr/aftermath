ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libaftermath-render.la

AM_CPPFLAGS = -Wall -Werror

libaftermath_render_la_SOURCES = src/cairo_extras.c \
	src/cairo_extras.h

libaftermath_render_la_CFLAGS = @CAIRO_CFLAGS@
libaftermath_render_la_LIBADD = @CAIRO_CFLAGS@

libaftermath_renderdir = $(includedir)/aftermath/render
libaftermath_render_HEADERS =

SUBDIRS = headers
