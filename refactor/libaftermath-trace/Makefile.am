ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libaftermath-trace.la

AM_CPPFLAGS = -I$(srcdir)/headers -I$(builddir)/headers -I$(builddir)/src

BUILT_SOURCES = \
	src/base_types.h

libaftermath_trace_la_SOURCES = \
	$(BUILT_SOURCES)

libaftermath_trace_la_CFLAGS =
libaftermath_trace_la_LIBADD =

libaftermath_tracedir = $(includedir)/aftermath/trace
libaftermath_trace_HEADERS =

src/base_types.h: @AFTERMATH_CORE_BUILDDIR@/src/base_types.h
	ln -sfr $< $@

SUBDIRS = headers

clean-local:
	rm -f $(BUILT_SOURCES)
