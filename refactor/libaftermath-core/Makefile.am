ACLOCAL_AMFLAGS=-I m4
AM_CPPFLAGS = -I$(srcdir)/headers -I$(builddir)/headers -Wall -Werror

lib_LTLIBRARIES = libaftermath-core.la

libaftermath_core_la_SOURCES = src/ansi_extras.c \
				src/ansi_extras.h \
				src/arithmetic.h \
				src/aux.h \
				src/base_types.h \
				src/bsearch.h \
				src/bits.h \
				src/bitvector.c \
				src/bitvector.h \
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
				src/counter_description_array.h \
				src/counter_event_array.h \
				src/counter_event_array_collection.h \
				src/default_event_array_registry.c \
				src/default_event_array_registry.h \
				src/dfg_builtin_node_impl.h \
				src/dfg_builtin_node_types.c \
				src/dfg_builtin_node_types.h \
				src/dfg_builtin_types.c \
				src/dfg_builtin_types.h \
				src/dfg_buffer.c \
				src/dfg_buffer.h \
				src/dfg_graph.c \
				src/dfg_graph.h \
				src/dfg_node.c \
				src/dfg_node.h \
				src/dfg_node_type_registry.c \
				src/dfg_node_type_registry.h \
				src/dfg_schedule.c \
				src/dfg_schedule.h \
				src/dfg_type.c \
				src/dfg_type.h \
				src/dfg_type_registry.c \
				src/dfg_type_registry.h \
				src/dfg/nodes/duration_to_string.c \
				src/dfg/nodes/duration_to_string.h \
				src/dfg/nodes/histogram_attributes.c \
				src/dfg/nodes/histogram_attributes.h \
				src/dfg/nodes/histogram_builder.c \
				src/dfg/nodes/histogram_builder.h \
				src/dfg/nodes/interval_attributes.c \
				src/dfg/nodes/interval_attributes.h \
				src/dfg/nodes/interval_duration.c \
				src/dfg/nodes/interval_duration.h \
				src/dfg/nodes/state_event_attributes.c \
				src/dfg/nodes/state_event_attributes.h \
				src/dfg/nodes/string_format.c \
				src/dfg/nodes/string_format.h \
				src/dfg/nodes/timestamp_to_string.c \
				src/dfg/nodes/timestamp_to_string.h \
				src/dfg/nodes/trace.c \
				src/dfg/nodes/trace.h \
				src/dfg/types/bool.c \
				src/dfg/types/bool.h \
				src/dfg/types/duration.c \
				src/dfg/types/duration.h \
				src/dfg/types/generic.c \
				src/dfg/types/generic.h \
				src/dfg/types/histogram.c \
				src/dfg/types/histogram.h \
				src/dfg/types/histogram_data.c \
				src/dfg/types/histogram_data.h \
				src/dfg/types/interval.c \
				src/dfg/types/interval.h \
				src/dfg/types/string.c \
				src/dfg/types/string.h \
				src/dfg/types/timestamp.c \
				src/dfg/types/timestamp.h \
				src/dfg/types/int.c \
				src/dfg/types/int.h \
				src/event_collection_array.h \
				src/event_collection.c \
				src/event_collection.h \
				src/event_mapping.c \
				src/event_mapping.h \
				src/hierarchy.c \
				src/hierarchy.h \
				src/hierarchy_array.h \
				src/interval.h \
				src/interval_array.h \
				src/in_memory.c \
				src/in_memory.h \
				src/io_context.c \
				src/io_context.h \
				src/io_error.c \
				src/io_error.h \
				src/io_hierarchy_context.c \
				src/io_hierarchy_context.h \
				src/measurement_interval_array.h \
				src/object_notation.c \
				src/object_notation.h \
				src/on_disk.c \
				src/on_disk.h \
				src/parser.h \
				src/ptr.h \
				src/safe_alloc.h \
				src/state_description_array.h \
				src/state_event_array.h \
				src/statistics/histogram.c \
				src/statistics/histogram.h \
				src/statistics/interval.c \
				src/statistics/interval.h \
				src/timestamp.h \
				src/trace.c \
				src/trace.h \
				src/typed_array.h \
				src/typed_list.h \
				src/typed_rbtree.h

libaftermath_core_la_CFLAGS =
libaftermath_core_la_LIBADD =

GENERATED_FILES=src/base_types.h \
		src/in_memory.c \
		src/in_memory.h \
		src/on_disk.c \
		src/on_disk.h

template_verbose = $(template_verbose_@AM_V@)
template_verbose_ = $(template_verbose_@AM_DEFAULT_V@)
template_verbose_0 = @echo "  TEMPLATE" $@;

TPL_DEPS=src/defs/am_base_types.py \
	src/defs/am_type_aux.py \
	src/defs/am_dsk_types.py \
	src/defs/am_mem_types.py \
	src/templates/dsk_load.tpl.c \
	src/templates/dsk_load.tpl.fnproto.h \
	src/templates/dsk_read.tpl.c \
	src/templates/dsk_read.tpl.fnproto.h \
	src/templates/dsk_read_fun.tpl.c \
	src/templates/dsk_to_mem_copy.tpl.c \
	src/templates/dsk_to_mem_copy.tpl.fnproto.h \
	src/templates/dsk_write.tpl.c \
	src/templates/dsk_write.tpl.fnproto.h \
	src/templates/destructor.tpl.c \
	src/templates/destructor.tpl.fnproto.h \
	src/templates/id_to_index.tpl.c \
	src/templates/id_to_index.tpl.fnproto.h \
	src/templates/id_to_index.tpl.fname.h \
	src/templates/per_event_collection_interval.tpl.c \
	src/templates/per_event_collection_per_id_timestamp.tpl.c \
	src/templates/per_trace_array.tpl.c \
	src/templates/process.tpl.fnproto.h \
	src/templates/timestamp_min_max_update.tpl.fnproto.h \
	src/templates/timestamp_min_max_update.tpl.c \
	./scripts/template.py

src/base_types.h: src/templates/base_types.tpl.h \
		$(TPL_DEPS)
	@$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/am_types.py \
		-t $(srcdir)/src/templates/base_types.tpl.h \
		-o $@

src/in_memory.c: src/templates/in_memory.tpl.c \
		$(TPL_DEPS)
	@$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/am_types.py \
		-t $(srcdir)/src/templates/in_memory.tpl.c \
		-o $@

src/in_memory.h: src/templates/in_memory.tpl.h \
		$(TPL_DEPS)
	@$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/am_types.py \
		-t $(srcdir)/src/templates/in_memory.tpl.h \
		-o $@

src/on_disk.c: src/templates/on_disk.tpl.c \
		$(TPL_DEPS)
	@$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/am_types.py \
		-t $(srcdir)/src/templates/on_disk.tpl.c \
		-o $@

src/on_disk.h: src/templates/on_disk.tpl.h \
		$(TPL_DEPS)
	@$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/am_types.py \
		-t $(srcdir)/src/templates/on_disk.tpl.h \
		-o $@

BUILT_SOURCES = $(GENERATED_FILES)

clean-local:
	rm -f $(GENERATED_FILES) src/defs/*.pyc

SUBDIRS = headers
