ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libaftermath-trace.la

AM_CPPFLAGS = -I$(srcdir)/headers -I$(builddir)/headers -I$(builddir)/src

BUILT_SOURCES = \
	src/base_types.h \
	src/convert.h \
	src/on_disk_default_type_ids.c \
	src/on_disk_default_type_ids.h \
	src/on_disk_structs.h \
	src/on_disk_write_to_buffer.h \
	src/safe_alloc.h \
	src/write_buffer.c \
	src/write_buffer.h

libaftermath_trace_la_SOURCES = \
	$(BUILT_SOURCES) \
	src/arch.h \
	src/buffered_event_collection.c \
	src/buffered_event_collection.h \
	src/buffered_trace.c \
	src/buffered_trace.h \
	src/simple_hierarchy.c \
	src/simple_hierarchy.h \
	src/state_stack.c \
	src/state_stack.h \
	src/timestamp.h

libaftermath_trace_la_CFLAGS =
libaftermath_trace_la_LIBADD =

libaftermath_tracedir = $(includedir)/aftermath/trace
libaftermath_trace_HEADERS =

src/base_types.h: @AFTERMATH_CORE_BUILDDIR@/src/base_types.h
	ln -sfr $< $@

src/convert.h: @AFTERMATH_CORE_SOURCEDIR@/src/convert.h
	ln -sfr $< $@

src/on_disk_default_type_ids.c: @AFTERMATH_CORE_BUILDDIR@/src/on_disk_default_type_ids.c
	ln -sfr $< $@

src/on_disk_default_type_ids.h: @AFTERMATH_CORE_BUILDDIR@/src/on_disk_default_type_ids.h
	ln -sfr $< $@

src/on_disk_structs.h: @AFTERMATH_CORE_BUILDDIR@/src/on_disk_structs.h
	ln -sfr $< $@

src/on_disk_write_to_buffer.h: @AFTERMATH_CORE_BUILDDIR@/src/on_disk_write_to_buffer.h
	ln -sfr $< $@

src/safe_alloc.h: @AFTERMATH_CORE_SOURCEDIR@/src/safe_alloc.h
	ln -sfr $< $@

src/write_buffer.c: @AFTERMATH_CORE_SOURCEDIR@/src/write_buffer.c
	ln -sfr $< $@

src/write_buffer.h: @AFTERMATH_CORE_SOURCEDIR@/src/write_buffer.h
	ln -sfr $< $@

SUBDIRS = headers

clean-local:
	rm -f $(BUILT_SOURCES)
