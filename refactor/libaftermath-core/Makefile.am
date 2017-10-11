ACLOCAL_AMFLAGS=-I m4
AM_CPPFLAGS = -I$(srcdir)/src

lib_LTLIBRARIES = libaftermath-core.la

libaftermath_core_la_SOURCES = src/ansi_extras.c \
				src/ansi_extras.h \
				src/base_types.h \
				src/bsearch.h \
				src/buffer.c \
				src/buffer.h \
				src/contrib/linux-kernel/kernel.h \
				src/contrib/linux-kernel/list.h \
				src/contrib/linux-kernel/poison.h \
				src/contrib/linux-kernel/rbtree_augmented.h \
				src/contrib/linux-kernel/rbtree.c \
				src/contrib/linux-kernel/rbtree.h \
				src/contrib/linux-kernel/stddef.h \
				src/contrib/linux-kernel/types.h \
				src/contrib/linux-kernel/userspace.h \
				src/convert.h \
				src/io_error.c \
				src/io_error.h \
				src/typed_array.h \
				src/typed_list.h \
				src/typed_rbtree.h

libaftermath_core_la_CFLAGS =
libaftermath_core_la_LIBADD =

GENERATED_FILES=src/base_types.h

template_verbose = $(template_verbose_@AM_V@)
template_verbose_ = $(template_verbose_@AM_DEFAULT_V@)
template_verbose_0 = @echo "  TEMPLATE" $@;

TPL_DEPS=src/defs/am_base_types.py \
	src/defs/am_type_aux.py
	./scripts/template.py

src/base_types.h: src/templates/base_types.tpl.h \
		$(TPL_DEPS)
	$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/am_types.py \
		-t $(srcdir)/src/templates/base_types.tpl.h \
		-o $@

BUILT_SOURCES = $(GENERATED_FILES)

clean-local:
	rm -f $(GENERATED_FILES) src/defs/*.pyc

SUBDIRS = headers
