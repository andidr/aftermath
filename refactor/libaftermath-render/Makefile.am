ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libaftermath-render.la

AM_CPPFLAGS = -Wall -Werror

libaftermath_render_la_SOURCES = src/cairo_extras.c \
	src/cairo_extras.h \
	src/timeline/layer.c \
	src/timeline/layer.h \
	src/timeline/layers/background.c \
	src/timeline/layers/background.h \
	src/timeline/layers/hierarchy.c \
	src/timeline/layers/hierarchy.h \
	src/timeline/renderer.c \
	src/timeline/renderer.h

libaftermath_render_la_CFLAGS = @AFTERMATH_CORE_INCLUDES@ @CAIRO_CFLAGS@
libaftermath_render_la_LIBADD = @AFTERMATH_CORE_LIBS@ @CAIRO_LIBS@

libaftermath_renderdir = $(includedir)/aftermath/render
libaftermath_render_HEADERS =

SUBDIRS = headers
