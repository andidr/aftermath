ACLOCAL_AMFLAGS=-I m4
AM_CPPFLAGS = -I$(srcdir)/headers -I$(builddir)/headers -Wall -Werror

lib_LTLIBRARIES = libaftermath-core.la

libaftermath_core_la_SOURCES = src/ansi_extras.c \
				src/ansi_extras.h \
				src/array_collection.c \
				src/array_collection.h \
				src/array_registry.c \
				src/array_registry.h \
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
				src/default_array_registry.h \
				src/default_event_collection_io_array_registry.c \
				src/default_event_collection_io_array_registry.h \
				src/default_trace_array_registry.c \
				src/default_trace_array_registry.h \
				src/dfg_builtin_node_impl.h \
				src/dfg_builtin_node_types.c \
				src/dfg_builtin_node_types.h \
				src/dfg_builtin_type_impl.h \
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
				src/dfg/nodes/bool_constant.c \
				src/dfg/nodes/bool_constant.h \
				src/dfg/nodes/count.c \
				src/dfg/nodes/count.h \
				src/dfg/nodes/double_constant.c \
				src/dfg/nodes/double_constant.h \
				src/dfg/nodes/duration_to_string.c \
				src/dfg/nodes/duration_to_string.h \
				src/dfg/nodes/duration_to_uint64.c \
				src/dfg/nodes/duration_to_uint64.h \
				src/dfg/nodes/event_mapping_common.h \
				src/dfg/nodes/hierarchy_attributes.c \
				src/dfg/nodes/hierarchy_attributes.h \
				src/dfg/nodes/hierarchy_node_attributes.c \
				src/dfg/nodes/hierarchy_node_attributes.h \
				src/dfg/nodes/hierarchy_node_selector.c \
				src/dfg/nodes/hierarchy_node_selector.h \
				src/dfg/nodes/hierarchy_selector.c \
				src/dfg/nodes/hierarchy_selector.h \
				src/dfg/nodes/histogram_attributes.c \
				src/dfg/nodes/histogram_attributes.h \
				src/dfg/nodes/histogram_builder.c \
				src/dfg/nodes/histogram_builder.h \
				src/dfg/nodes/int_constant.c \
				src/dfg/nodes/int_constant.h \
				src/dfg/nodes/int_to_string.c \
				src/dfg/nodes/int_to_string.h \
				src/dfg/nodes/interval_attributes.c \
				src/dfg/nodes/interval_attributes.h \
				src/dfg/nodes/interval_duration.c \
				src/dfg/nodes/interval_duration.h \
				src/dfg/nodes/merge.c \
				src/dfg/nodes/merge.h \
				src/dfg/nodes/pair_timestamp_hierarchy_node_attributes.c \
				src/dfg/nodes/pair_timestamp_hierarchy_node_attributes.h \
				src/dfg/nodes/state_description_attributes.c \
				src/dfg/nodes/state_description_attributes.h \
				src/dfg/nodes/state_event_attributes.c \
				src/dfg/nodes/state_event_attributes.h \
				src/dfg/nodes/string_concat.c \
				src/dfg/nodes/string_concat.h \
				src/dfg/nodes/string_constant.c \
				src/dfg/nodes/string_constant.h \
				src/dfg/nodes/string_format.c \
				src/dfg/nodes/string_format.h \
				src/dfg/nodes/timestamp_to_string.c \
				src/dfg/nodes/timestamp_to_string.h \
				src/dfg/nodes/timestamp_to_uint64.c \
				src/dfg/nodes/timestamp_to_uint64.h \
				src/dfg/nodes/tensorflow_node_attributes.c \
				src/dfg/nodes/tensorflow_node_attributes.h \
				src/dfg/nodes/tensorflow_node_execution_attributes.c \
				src/dfg/nodes/tensorflow_node_execution_attributes.h \
				src/dfg/nodes/trace.c \
				src/dfg/nodes/trace.h \
				src/dfg/types/any.h \
				src/dfg/types/bool.c \
				src/dfg/types/bool.h \
				src/dfg/types/double.c \
				src/dfg/types/double.h \
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
				src/dfg/types/pair_timestamp_hierarchy_node.h \
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
				src/frame_type_registry.c \
				src/frame_type_registry.h \
				src/frame_type_registry.priv.h \
				src/hierarchy.c \
				src/hierarchy.h \
				src/hierarchy_array.h \
				src/index_to_id_map.h \
				src/indexes/kdtree.c \
				src/indexes/kdtree.h \
				src/indexes/recttree.c \
				src/indexes/recttree.h \
				src/interval.c \
				src/interval.h \
				src/interval_array.h \
				src/in_memory.c \
				src/in_memory.h \
				src/in_memory_dfg_node_types.h \
				src/in_memory_dfg_node_types.c \
				src/in_memory_dfg_types.h \
				src/in_memory_inline.h \
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
				src/on_disk_default_type_ids.c \
				src/on_disk_default_type_ids.h \
				src/on_disk_meta.h \
				src/on_disk_structs.h \
				src/on_disk_write_to_buffer.h \
				src/openmp.h \
				src/openmp_for_loop_instance_array.h \
				src/openmp_for_loop_type_array.h \
				src/openmp_iteration_period_array.h \
				src/openmp_iteration_set_array.h \
				src/openmp_task_instance_array.h \
				src/openmp_task_period_array.h \
				src/openmp_task_type_array.h \
				src/openstream_task_instance_array.h \
				src/openstream_task_period_array.h \
				src/openstream_task_type_array.h \
				src/parser.h \
				src/prng.h \
				src/ptr.h \
				src/qsort.h \
				src/safe_alloc.h \
				src/state_description_array.h \
				src/state_event_array.h \
				src/statistics/histogram.c \
				src/statistics/histogram.h \
				src/statistics/interval.c \
				src/statistics/interval.h \
				src/tensorflow_node_array.h \
				src/tensorflow_node_execution_array.h \
				src/timestamp.h \
				src/trace.c \
				src/trace.h \
				src/typed_array.h \
				src/typed_list.h \
				src/typed_rbtree.h \
				src/write_buffer.c \
				src/write_buffer.h

libaftermath_core_la_CFLAGS =
libaftermath_core_la_LIBADD =

GENERATED_FILES=src/base_types.h \
		src/in_memory.c \
		src/in_memory.h \
		src/in_memory_dfg_node_types.c \
		src/in_memory_dfg_node_types.h \
		src/in_memory_dfg_types.h \
		src/in_memory_inline.h \
		src/on_disk.c \
		src/on_disk.h \
		src/on_disk_default_type_ids.c \
		src/on_disk_default_type_ids.h \
		src/on_disk_meta.h \
		src/on_disk_structs.h \
		src/on_disk_write_to_buffer.h

template_verbose = $(template_verbose_@AM_V@)
template_verbose_ = $(template_verbose_@AM_DEFAULT_V@)
template_verbose_0 = @echo "  TEMPLATE" $@;

TPL_DEPS=src/defs/aftermath/__init__.py \
	src/defs/aftermath/relations/__init__.py \
	src/defs/aftermath/relations/collect.py \
	src/defs/aftermath/relations/join.py \
	src/defs/aftermath/tags/assertion.py \
	src/defs/aftermath/tags/dsk/__init__.py \
	src/defs/aftermath/tags/dsk/tomem/__init__.py \
	src/defs/aftermath/tags/dsk/tomem/join.py \
	src/defs/aftermath/tags/finalize.py \
	src/defs/aftermath/tags/__init__.py \
	src/defs/aftermath/tags/mem/__init__.py \
	src/defs/aftermath/tags/mem/collect.py \
	src/defs/aftermath/tags/mem/dfg.py \
	src/defs/aftermath/tags/mem/store/__init__.py \
	src/defs/aftermath/tags/mem/store/pereventcollectionarray.py \
	src/defs/aftermath/tags/mem/store/pereventcollectionsubarray.py \
	src/defs/aftermath/tags/mem/store/pertracearray.py \
	src/defs/aftermath/tags/postprocess/__init__.py \
	src/defs/aftermath/tags/process/__init__.py \
	src/defs/aftermath/tags/teardown.py \
	src/defs/aftermath/templates/assertion/AssertFunction.tpl.c \
	src/defs/aftermath/templates/assertion/__init__.py \
	src/defs/aftermath/templates/dsk/DumpStdoutFunction.tpl.c \
	src/defs/aftermath/templates/dsk/__init__.py \
	src/defs/aftermath/templates/dsk/LoadFunction.tpl.c \
	src/defs/aftermath/templates/dsk/ProcessFunction.tpl.c \
	src/defs/aftermath/templates/dsk/ReadFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/AddToAllMetaStructArraysFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/ConversionFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/__init__.py \
	src/defs/aftermath/templates/dsk/tomem/MatchAllMetaStructArraysFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/PerEventCollectionArrayFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/PerEventCollectionSubArrayFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/PerTraceArrayFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/PostConversionFunction.tpl.c \
	src/defs/aftermath/templates/dsk/tomem/SortMetaStructArrayFunction.tpl.c \
	src/defs/aftermath/templates/dsk/WriteDefaultIDFunction.tpl.c \
	src/defs/aftermath/templates/dsk/WriteDefaultIDToBufferFunction.tpl.c \
	src/defs/aftermath/templates/dsk/WriteFunction.tpl.c \
	src/defs/aftermath/templates/dsk/WriteWithDefaultIDFunction.tpl.c \
	src/defs/aftermath/templates/dsk/WriteToBufferFunction.tpl.c \
	src/defs/aftermath/templates/dsk/WriteToBufferWithDefaultIDFunction.tpl.c \
	src/defs/aftermath/templates/finalize/FinalizeFunction.tpl.c \
	src/defs/aftermath/templates/finalize/__init__.py \
	src/defs/aftermath/templates/__init__.py \
	src/defs/aftermath/templates/mem/__init__.py \
	src/defs/aftermath/templates/mem/GenerateProcessCollectSourcesFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/__init__.py \
	src/defs/aftermath/templates/mem/store/pereventcollectionarray/AppendAndSetIndexFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pereventcollectionarray/AppendFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pereventcollectionarray/DestroyAllArraysFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pereventcollectionarray/__init__.py \
	src/defs/aftermath/templates/mem/store/pereventcollectionsubarray/AppendAndSetIndexFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pereventcollectionsubarray/AppendFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pereventcollectionsubarray/DestroyAllArraysFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pereventcollectionsubarray/__init__.py \
	src/defs/aftermath/templates/mem/store/pertracearray/AppendAndSetIndexFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pertracearray/AppendFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pertracearray/DestroyAllArraysFunction.tpl.c \
	src/defs/aftermath/templates/mem/store/pertracearray/__init__.py \
	src/defs/aftermath/templates/postprocess/__init__.py \
	src/defs/aftermath/templates/postprocess/PostprocessFunction.tpl.c \
	src/defs/aftermath/templates/process/__init__.py \
	src/defs/aftermath/templates/process/TraceMinMaxTimestampCompoundScan.tpl.c \
	src/defs/aftermath/templates/teardown/__init__.py \
	src/defs/aftermath/templates/teardown/TeardownFunction.tpl.c \
	src/defs/aftermath/templates/toplevel/base_types.tpl.h \
	src/defs/aftermath/templates/toplevel/defs.py \
	src/defs/aftermath/templates/toplevel/in_memory_dfg_node_types.tpl.c \
	src/defs/aftermath/templates/toplevel/in_memory_dfg_node_types.tpl.h \
	src/defs/aftermath/templates/toplevel/in_memory_dfg_types.tpl.h \
	src/defs/aftermath/templates/toplevel/in_memory_inline.tpl.h \
	src/defs/aftermath/templates/toplevel/in_memory.tpl.c \
	src/defs/aftermath/templates/toplevel/in_memory.tpl.h \
	src/defs/aftermath/templates/toplevel/on_disk_default_type_ids.tpl.c \
	src/defs/aftermath/templates/toplevel/on_disk_default_type_ids.tpl.h \
	src/defs/aftermath/templates/toplevel/on_disk_meta.tpl.h \
	src/defs/aftermath/templates/toplevel/on_disk_structs.tpl.h \
	src/defs/aftermath/templates/toplevel/on_disk_write_to_buffer.tpl.h \
	src/defs/aftermath/templates/toplevel/on_disk.tpl.c \
	src/defs/aftermath/templates/toplevel/on_disk.tpl.h \
	src/defs/aftermath/types/aux.py \
	src/defs/aftermath/types/base.py \
	src/defs/aftermath/types/builtin.py \
	src/defs/aftermath/types/__init__.py \
	src/defs/aftermath/types/in_memory.py \
	src/defs/aftermath/types/on_disk.py \
	src/defs/aftermath/types/openmp/__init__.py \
	src/defs/aftermath/types/openmp/in_memory.py \
	src/defs/aftermath/types/openmp/on_disk.py \
	src/defs/aftermath/types/openstream/__init__.py \
	src/defs/aftermath/types/openstream/in_memory.py \
	src/defs/aftermath/types/openstream/on_disk.py \
	src/defs/aftermath/types/tensorflow/__init__.py \
	src/defs/aftermath/types/tensorflow/in_memory.py \
	src/defs/aftermath/types/tensorflow/on_disk.py \
	src/defs/aftermath/util/__init__.py

PYC_FILES=src/defs/aftermath/__init__.pyc \
	src/defs/aftermath/relations/__init__.pyc \
	src/defs/aftermath/relations/collect.pyc \
	src/defs/aftermath/relations/join.pyc \
	src/defs/aftermath/tags/assertion.pyc \
	src/defs/aftermath/tags/dsk/__init__.pyc \
	src/defs/aftermath/tags/dsk/tomem/__init__.pyc \
	src/defs/aftermath/tags/dsk/tomem/join.pyc \
	src/defs/aftermath/tags/finalize.pyc \
	src/defs/aftermath/tags/__init__.pyc \
	src/defs/aftermath/tags/mem/__init__.pyc \
	src/defs/aftermath/tags/mem/collect.pyc \
	src/defs/aftermath/tags/mem/dfg.pyc \
	src/defs/aftermath/tags/mem/store/__init__.pyc \
	src/defs/aftermath/tags/mem/store/pereventcollectionarray.pyc \
	src/defs/aftermath/tags/mem/store/pereventcollectionsubarray.pyc \
	src/defs/aftermath/tags/mem/store/pertracearray.pyc \
	src/defs/aftermath/tags/postprocess/__init__.pyc \
	src/defs/aftermath/tags/process/__init__.pyc \
	src/defs/aftermath/tags/teardown.pyc \
	src/defs/aftermath/templates/assertion/__init__.pyc \
	src/defs/aftermath/templates/dsk/__init__.pyc \
	src/defs/aftermath/templates/dsk/tomem/__init__.pyc \
	src/defs/aftermath/templates/finalize/__init__.pyc \
	src/defs/aftermath/templates/__init__.pyc \
	src/defs/aftermath/templates/mem/__init__.pyc \
	src/defs/aftermath/templates/mem/store/__init__.pyc \
	src/defs/aftermath/templates/mem/store/pereventcollectionarray/__init__.pyc \
	src/defs/aftermath/templates/mem/store/pereventcollectionsubarray/__init__.pyc \
	src/defs/aftermath/templates/mem/store/pertracearray/__init__.pyc \
	src/defs/aftermath/templates/postprocess/__init__.pyc \
	src/defs/aftermath/templates/process/__init__.pyc \
	src/defs/aftermath/templates/teardown/__init__.pyc \
	src/defs/aftermath/templates/toplevel/defs.pyc \
	src/defs/aftermath/types/aux.pyc \
	src/defs/aftermath/types/base.pyc \
	src/defs/aftermath/types/builtin.pyc \
	src/defs/aftermath/types/__init__.pyc \
	src/defs/aftermath/types/in_memory.pyc \
	src/defs/aftermath/types/on_disk.pyc \
	src/defs/aftermath/types/openmp/__init__.pyc \
	src/defs/aftermath/types/openmp/in_memory.pyc \
	src/defs/aftermath/types/openmp/on_disk.pyc \
	src/defs/aftermath/types/openstream/__init__.pyc \
	src/defs/aftermath/types/openstream/in_memory.pyc \
	src/defs/aftermath/types/openstream/on_disk.pyc \
	src/defs/aftermath/types/tensorflow/__init__.pyc \
	src/defs/aftermath/types/tensorflow/in_memory.pyc \
	src/defs/aftermath/types/tensorflow/on_disk.pyc \
	src/defs/aftermath/util/__init__.pyc

src/base_types.h: src/defs/aftermath/templates/toplevel/base_types.tpl.h $(TPL_DEPS)
src/in_memory.h: src/defs/aftermath/templates/toplevel/in_memory.tpl.h $(TPL_DEPS)
src/in_memory.c: src/defs/aftermath/templates/toplevel/in_memory.tpl.c $(TPL_DEPS)
src/in_memory_dfg_node_types.c: src/defs/aftermath/templates/toplevel/in_memory_dfg_node_types.tpl.c $(TPL_DEPS)
src/in_memory_dfg_node_types.h: src/defs/aftermath/templates/toplevel/in_memory_dfg_node_types.tpl.h $(TPL_DEPS)
src/in_memory_dfg_types.h: src/defs/aftermath/templates/toplevel/in_memory_dfg_types.tpl.h $(TPL_DEPS)
src/in_memory_inline.h: src/defs/aftermath/templates/toplevel/in_memory_inline.tpl.h $(TPL_DEPS)
src/on_disk.h: src/defs/aftermath/templates/toplevel/on_disk.tpl.h $(TPL_DEPS)
src/on_disk.c: src/defs/aftermath/templates/toplevel/on_disk.tpl.c $(TPL_DEPS)
src/on_disk_default_type_ids.c: src/defs/aftermath/templates/toplevel/on_disk_default_type_ids.tpl.c $(TPL_DEPS)
src/on_disk_default_type_ids.h: src/defs/aftermath/templates/toplevel/on_disk_default_type_ids.tpl.h $(TPL_DEPS)
src/on_disk_meta.h: src/defs/aftermath/templates/toplevel/on_disk_meta.tpl.h $(TPL_DEPS)
src/on_disk_structs.h: src/defs/aftermath/templates/toplevel/on_disk_structs.tpl.h $(TPL_DEPS)
src/on_disk_write_to_buffer.h: src/defs/aftermath/templates/toplevel/on_disk_write_to_buffer.tpl.h $(TPL_DEPS)

$(GENERATED_FILES):
	@$(MKDIR_P) src/generated
	$(template_verbose)$(PYTHON) ./scripts/template.py \
		-m $(srcdir)/src/defs/aftermath/templates/toplevel/defs.py \
		-t $< \
		-o $@

BUILT_SOURCES = $(GENERATED_FILES)

TAGS_FILES = $(TPL_DEPS)

if HAVE_DOXYGEN
# Doxygen does not seem to be capable of taking into account
# files that do not contain "@file". Also, comments are
# ignored unless starting with /** etc.
#
# This rule copies the entire source tree to
# doc/doxygen/transformed-source, adds a "/** @file */" at the
# beginning if needed and transforms all ordinary comments into
# Doxygen ones.
#
# WARNING: All modifications are done by simple text replacement; no
# syntax analysis is done on the processes sources, which might result
# in erroneous replacements.
doxygen-doc: doc/Doxyfile
	rm -rf doc/doxygen/transformed-src
	$(MKDIR_P) doc/doxygen/transformed-src
	( find "@srcdir@/src" -type f \( -iname "*.c" -o -iname "*.h" -o -iname "*.cpp" -o -iname "*.hpp" -o -iname "*.py" \) ; \
		find "@builddir@/src" -type f \( -iname "*.c" -o -iname "*.h" -o -iname "*.cpp" -o -iname "*.hpp" -o -iname "*.py" \) ) \
		| sort | uniq > doxygen-files.list
	tar c -T doxygen-files.list | ( cd doc/doxygen/transformed-src ; tar x )
	rm -f doxygen-files.list
	cd doc/doxygen/transformed-src ; \
		for FILE in `find -type f \( -iname "*.c" -o -iname "*.h" -o -iname "*.cpp" -o -iname "*.hpp" \) ` ; \
		do \
			if $(GREP) "@file" "$$FILE" ; \
			then \
				true ; \
			else \
				printf "/**\n* @file\n*/\n" > "$$FILE.bak" ; \
				cat "$$FILE" >> "$$FILE.bak" ; \
				mv "$$FILE.bak" "$$FILE" ; \
			fi ; \
			\
			sed -i -e 's|/\*\([^*]\)|/**\1|g' "$$FILE" \
				-e 's|//\([^/]\)|///\1|g' "$$FILE" ; \
		done
	$(DOXYGEN) doc/Doxyfile
	rm -rf doc/doxygen/transformed-src
endif

clean-local:
	rm -f $(GENERATED_FILES) $(PYC_FILES)
	rm -rf $(builddir)/doc/doxygen
	rm -rf tmp/packaging

SUBDIRS = headers
EXTRA_DIST = headers scripts src/defs

UPSTREAM_VERSION=@PACKAGE_VERSION@

deb:
	$(MAKE) dist-gzip
	rm -rf tmp/packaging/debian
	$(MKDIR_P) tmp/packaging/debian
	mv libaftermath-core-$(UPSTREAM_VERSION).tar.gz tmp/packaging/debian/libaftermath-core_$(UPSTREAM_VERSION).orig.tar.gz
	cd tmp/packaging/debian ; tar xf libaftermath-core_$(UPSTREAM_VERSION).orig.tar.gz
	cp -ar $(srcdir)/distros/debian/build/debian tmp/packaging/debian/libaftermath-core-$(UPSTREAM_VERSION)
	cd tmp/packaging/debian/libaftermath-core-$(UPSTREAM_VERSION) ; debuild -us -uc
