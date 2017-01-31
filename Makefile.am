ACLOCAL_AMFLAGS=-I m4
VALGRIND=valgrind
bin_PROGRAMS = aftermath
bin_SCRIPTS = aftermath-debug-syms

DISTRO_EXTRA_DIST = distros

info_TEXINFOS = doc/texinfo/openmp-shortguide.texi

TEXINFO_IMAGES = doc/texinfo/images/aftermath-omp-prime_naive_guided-tlzoom-constructs.png \
	doc/texinfo/images/aftermath-omp-prime_naive_tasks-stats.png \
	doc/texinfo/images/aftermath-omp-prime_naive_tasks-tlzoom-tasks.png \
	doc/texinfo/images/aftermath-omp-prime_naive-loop-construct-selection.png \
	doc/texinfo/images/aftermath-omp-prime_naive-loop-construct.png \
	doc/texinfo/images/aftermath-omp-prime_naive-cpu1.png \
	doc/texinfo/images/aftermath-omp-prime_naive-tlzoom-loops.png \
	doc/texinfo/images/aftermath-omp-prime_naive-tlzoom-constructs.png \
	doc/texinfo/images/aftermath-omp-prime_naive-tlzoom-itersets.png \
	doc/texinfo/images/aftermath-omp-prime_naive-cpu2.png \
	doc/texinfo/images/aftermath-omp-prime_naive_guided-tlzoom-itersets.png \
	doc/texinfo/images/aftermath-omp-prime_naive-cpu0.png \
	doc/texinfo/images/aftermath-omp-prime_naive_dynamic-detail.png \
	doc/texinfo/images/aftermath-omp-prime_naive_tasks-tlzoom-states-end.png \
	doc/texinfo/images/aftermath-omp-prime_naive_tasks-tlzoom-states.png \
	doc/texinfo/images/aftermath-omp-prime_naive-cpu3.png \
	doc/texinfo/images/aftermath-omp-prime_naive_tasks-tlzoom-constructs.png \
	doc/texinfo/images/aftermath-omp-prime_naive_dynamic-tlzoom-constructs.png \
	doc/texinfo/images/aftermath-omp-prime_naive_guided-detail.png \
	doc/texinfo/images/aftermath-omp-prime_naive_tasks-detail.png \
	doc/texinfo/images/aftermath-omp-prime_naive-tlzoom-iterperiods.png \
	doc/texinfo/images/aftermath-toolbar-loop-modes.png \
	doc/texinfo/images/aftermath-omp-prime_naive_dynamic-tlzoom-itersets.png

TEXINFO_EXTRA_DIST = doc/texinfo/images \
	doc/texinfo/fdl.texi

html-local:
	$(MKDIR_P) doc/texinfo/openmp-shortguide.html/images
	cp -ar doc/texinfo/images/* doc/texinfo/openmp-shortguide.html/images

install-info-local:
	echo " $(MKDIR_P) '$(DESTDIR)$(infodir)/images'"; \
		$(MKDIR_P) '$(DESTDIR)$(infodir)/images' || exit 1;
	for img in $(TEXINFO_IMAGES); \
	do \
		$(INSTALL_DATA) "$(srcdir)/$$img" '$(DESTDIR)$(infodir)/images/' || exit $$? ; \
	done

uninstall-local:
	for img in $(TEXINFO_IMAGES); \
	do \
		rm -f "$(DESTDIR)$(infodir)/images/`basename $$img`" || exit $$? ; \
	done

dvi:
	@echo "DVI output has been disabled. Please use the pdf or the html target"

lib_LTLIBRARIES = libaftermath-core.la \
		libaftermath-trace.la

VALGRIND_FLAGS=--gen-suppressions=all \
		--track-origins=yes \
		--leak-check=full \
		--show-reachable=yes \
		--leak-resolution=high \
		--error-exitcode=1 \
		--suppressions=./tests/valgrind-suppressions \
		--run-libc-freeres=no \
		--partial-loads-ok=yes \
		--fullpath-after=

libaftermath_core_la_SOURCES = src/buffer.h \
		src/buffer.c \
		src/color.h \
		src/color.c \
		src/events.h \
		src/events.c \
		src/filter.h \
		src/filter.c \
		src/filter_expression.h \
		src/filter_expression.c \
		src/trace/trace_file.h \
		src/trace/trace_file.c \
		src/debug.h \
		src/debug.c \
		src/color_scheme.h \
		src/color_scheme.c \
		src/settings.h \
		src/settings.c \
		src/bitvector.h \
		src/bitvector.c \
		src/counter_description.h \
		src/state_description.h \
		src/state_description.c \
		src/counter_event_set.h \
		src/counter_event_set.c \
		src/counter_event_set_index.h \
		src/counter_event_set_index.c \
		src/event_set.h \
		src/event_set.c \
		src/multi_event_set.h \
		src/multi_event_set.c \
		src/task.h \
		src/task.c \
		src/frame.h \
		src/frame.c \
		src/util.h \
		src/util.c \
		src/derived_counters.h \
		src/derived_counters.c \
		src/page.h \
		src/page.c \
		src/ansi_extras.h \
		src/ansi_extras.c \
		src/annotation.h \
		src/annotation.c \
		src/visuals_file.h \
		src/visuals_file.c \
		src/trace/convert.h \
		src/trace/convert.c \
		src/task_graph.h \
		src/task_graph.c \
		src/uncompress.h \
		src/uncompress.c \
		src/contrib/linux-kernel/interval_tree_generic.h \
		src/contrib/linux-kernel/kernel.h \
		src/contrib/linux-kernel/list.h \
		src/contrib/linux-kernel/poison.h \
		src/contrib/linux-kernel/rbtree_augmented.h \
		src/contrib/linux-kernel/rbtree.h \
		src/contrib/linux-kernel/rbtree.c \
		src/contrib/linux-kernel/stddef.h \
		src/contrib/linux-kernel/types.h \
		src/contrib/linux-kernel/userspace.h \
		src/typed_list.h \
		src/typed_array.h \
		src/address_range_tree.h \
		src/address_range_tree.c \
		src/task_instance.h \
		src/task_instance.c \
		src/task_instance_rw_tree.h \
		src/task_instance_rw_tree.c \
		src/task_instance_tree.h \
		src/task_instance_tree.c \
		src/omp_for.h \
		src/omp_for.c \
		src/omp_for_instance.h \
		src/omp_for_instance.c \
		src/omp_for_chunk_set.h \
		src/omp_for_chunk_set.c \
		src/omp_for_chunk_set_part.h \
		src/omp_for_chunk_set_part.c \
		src/omp_task.h \
		src/omp_task.c \
		src/omp_task_instance.h \
		src/omp_task_instance.c \
		src/omp_task_part.h \
		src/omp_task_part.c \
		src/aux.h \
		src/parser.h \
		src/object_notation.h \
		src/object_notation.c \
		src/monotonic_index.h \
		src/monotonic_index.c \
		src/state_index.h \
		src/state_index.c \
		src/intensity_matrix.h \
		src/intensity_matrix.c \
		src/statistics.h \
		src/statistics.c \
		src/hierarchy.h \
		src/hierarchy.c

libaftermath_core_la_CFLAGS = @GLIB_CFLAGS@
libaftermath_core_la_LIBADD = @GLIB_LIBS@ -lm

libaftermath_coredir = $(includedir)/aftermath/core
libaftermath_core_HEADERS = $(filter %.h,$(libaftermath_core_la_SOURCES))

libaftermath_trace_la_SOURCES = src/trace/trace.h \
				src/trace/trace.c \
				src/trace/types.h \
				src/trace/ansi_extras.h \
				src/trace/convert.h \
				src/trace/convert.c \
				src/trace/trace_file.h \
				src/trace/trace_file.c \
				src/trace/arch.h \
				src/trace/state_stack.h \
				src/trace/state_stack.c \
				src/trace/tsc.h \
				src/trace/tsc.c

libaftermath_trace_la_LIBADD = -lrt

libaftermath_tracedir = $(includedir)/aftermath/trace
libaftermath_trace_HEADERS = src/trace/trace.h \
			src/trace/types.h \
			src/trace/arch.h \
			src/trace/state_stack.h \
			src/trace/tsc.h

aftermath_SOURCES = src/aftermath.c \
		src/trace_widget.h \
		src/trace_widget.c \
		src/histogram_widget.h \
		src/histogram_widget.c \
		src/multi_histogram_widget.h \
		src/multi_histogram_widget.c \
		src/matrix_widget.h \
		src/matrix_widget.c \
		src/signals.h \
		src/signals.c \
		src/globals.h \
		src/globals.c \
		src/marshal.in \
		src/generated/marshal.h \
		src/generated/marshal.c \
		src/dialogs.h \
		src/dialogs.c \
		src/task_list.h \
		src/task_list.c \
		src/omp_for_treeview.h \
		src/omp_for_treeview.c \
		src/omp_task_treeview.h \
		src/omp_task_treeview.c \
		src/frame_list.h \
		src/frame_list.c \
		src/counter_list.h \
		src/counter_list.c \
		src/state_list.h \
		src/state_list.c \
		src/color_scheme_list.h \
		src/color_scheme_list.c \
		src/color_scheme_set_list.h \
		src/color_scheme_set_list.c \
		src/color.h \
		src/color.c \
		src/cell_renderer_color_button.h \
		src/cell_renderer_color_button.c \
		src/glade_extras.h \
		src/numa_node_list.h \
		src/numa_node_list.c \
		src/cpu_list.h \
		src/cpu_list.c \
		src/cairo_extras.h \
		src/cairo_extras.c \
		src/gtk_extras.h \
		src/gtk_extras.c \
		src/export.h \
		src/aftermath-debug-syms.sh \
		doc/man/aftermath.1 \
		$(UNIT_TEST_SOURCES)

aftermath_CFLAGS = $(TESTS_CFLAGS) @GTK_CFLAGS@ @GLADE_CFLAGS@ -DDATA_PATH="\"${datarootdir}/aftermath\""
aftermath_LDADD = libaftermath-core.la $(TESTS_LDADD) @GTK_LIBS@ @GLADE_LIBS@
aftermath_LDFLAGS = -export-dynamic
aftermathdir=${datarootdir}/aftermath
dist_aftermath_DATA = share/ui/aftermath.glade \
		share/ui/goto_dialog.glade \
		share/ui/select_interval_dialog.glade \
		share/ui/about_dialog.glade \
		share/ui/settings.glade \
		share/ui/progress_dialog.glade \
		share/ui/color_dialog.glade \
		share/ui/derived_counter_dialog.glade \
		share/ui/annotation_dialog.glade \
		share/ui/counter_offset_dialog.glade \
		share/ui/task_graph_texec_dialog.glade \
		share/ui/parallelism_dialog.glade \
		share/ui/message_dialog.glade \
		share/ui/color_schemes_dialog.glade \
		share/ui/filter_expression_dialog.glade \
		share/icons/draw_states.svg \
		share/icons/draw_steals.svg \
		share/icons/draw_pushes.svg \
		share/icons/draw_reads.svg \
		share/icons/draw_writes.svg \
		share/icons/draw_size.svg \
		share/icons/draw_events.svg \
		share/icons/draw_counters.svg \
		share/icons/draw_annotations.svg \
		share/icons/draw_predecessors.svg \
		share/icons/heatmap.svg \
		share/icons/typemap.svg \
		share/icons/numamap_reads.svg \
		share/icons/numamap_writes.svg \
		share/icons/heatmap_numa.svg \
		share/icons/fit_all_cpus.svg \
		share/icons/fit_all_measurement_intervals.svg \
		share/icons/draw_measurement_intervals.svg \
		share/icons/omp_for.svg \
		share/bitmap/splash.png

man_MANS = doc/man/aftermath.1

clean-local:
	-find . -iname "*~" -delete
	rm -f tests/wr_trace2core.test.c

%-silent:
	@$(MAKE) $* | EGREP="$(EGREP)" CC="$(CC)" SED="$(SED)" AR="$(AR)" ./scripts/build_silently_filter.sh

valgrind-check: $(foreach TEST,$(TESTS),valgrind-check-$(TEST))

check-%: %
	@./$<

valgrind-check-raw-%: %
	@echo -n "[VALGRIND]"
	@G_SLICE=always-malloc G_DEBUG=gc-friendly \
	    libtool --mode=execute \
		$(VALGRIND) $(VALGRIND_FLAGS) ./$< 2> valgrind.$<.log ; \
	if [ $$? != 0 ] ; \
	then \
		cat valgrind.$<.log; \
		exit 1; \
	fi; \
	./tests/lib/valgrind_check_error.sh valgrind.$<.log ; \
	if [ $$? != 0 ] ; \
	then \
		cat valgrind.$<.log; \
		exit 1; \
	fi; \
	if [ -z $$KEEP_VALGRIND_LOG ] ; \
	then \
		rm valgrind.$<.log; \
	fi ;

valgrind-check-%: % check-%
	$(MAKE) valgrind-check-raw-$<

whitespace-cleanup:
	@./scripts/whitespace_cleanup.sh

src/generated/marshal.h: src/marshal.in
	glib-genmarshal --header $< > $@

src/generated/marshal.c: src/marshal.in
	glib-genmarshal --body $< > $@

src/trace_widget.c: src/generated/marshal.h
src/trace_widget.h: src/generated/marshal.h

aftermath-debug-syms: src/aftermath-debug-syms.sh
	cp $< $@
	chmod +x $@

CLEANFILES = aftermath-debug-syms aftermath.1 scripts/check-header.sh

TESTS_LDADD = -L. -lm -lpthread libaftermath-core.la
TESTS_CFLAGS = -g -O0 $(LIB_CFLAGS) -D_POSIX_C_SOURCE=200809 -I$(srcdir)/tests/lib -I$(srcdir)/src

UNIT_TEST_SOURCES = tests/lib/unit_tests.c tests/lib/unit_tests.h tests/common.h
UNIT_TEST_EXTRA_DIST = tests/auto/scripts/hdict.py \
			tests/auto/scripts/rwspec_helpers.py \
			tests/auto/scripts/rwspec_defs.py \
			tests/auto/scripts/rwspec.py \
			tests/auto/read-write/states.rwspec \
			tests/auto/read-write/counters.rwspec \
			tests/auto/read-write/measurement_intervals.rwspec

TESTS = counter_event_set_index_test \
	event_set_test \
	wr_trace_states_test \
	wr_trace2core_test \
	ansi_extras_test \
	parser_test \
	object_notation_test \
	color_scheme_test \
	bitvector_test \
	filter_expression_test \
	monotonic_index_test \
	state_index_test \
	hierarchy_test \
	typed_array_test

noinst_PROGRAMS = $(TESTS)

counter_event_set_index_test_SOURCES = tests/counter_event_set_index.test.c \
	$(UNIT_TEST_SOURCES)

counter_event_set_index_test_LDADD = $(TESTS_LDADD)
counter_event_set_index_test_CFLAGS = $(TESTS_CFLAGS)

event_set_test_SOURCES = tests/event_set.test.c \
	$(UNIT_TEST_SOURCES)

event_set_test_CFLAGS = $(TESTS_CFLAGS)
event_set_test_LDADD = $(TESTS_LDADD)

wr_trace_states_test_SOURCES = tests/wr_trace_states.test.c \
	$(UNIT_TEST_SOURCES)

wr_trace_states_test_CFLAGS = $(TESTS_CFLAGS)
wr_trace_states_test_LDADD = $(TESTS_LDADD)

tests/wr_trace2core.test.c: ./tests/auto/scripts/rwspec.py \
			./tests/auto/read-write/states.rwspec \
			./tests/auto/read-write/counters.rwspec \
			./tests/auto/read-write/measurement_intervals.rwspec
	python $(srcdir)/tests/auto/scripts/rwspec.py -o $@ $(filter %.rwspec,$^)

wr_trace2core_test_SOURCES = tests/wr_trace2core.test.c \
	$(UNIT_TEST_SOURCES)

wr_trace2core_test_CFLAGS = $(TESTS_CFLAGS) -Isrc
wr_trace2core_test_LDADD = $(TESTS_LDADD) libaftermath-trace.la

ansi_extras_test_SOURCES = tests/ansi_extras.test.c \
	$(UNIT_TEST_SOURCES)

ansi_extras_test_CFLAGS = $(TESTS_CFLAGS)
ansi_extras_test_LDADD = $(TESTS_LDADD)

parser_test_SOURCES = tests/parser.test.c \
	$(UNIT_TEST_SOURCES)

parser_test_CFLAGS = $(TESTS_CFLAGS)
parser_test_LDADD = $(TESTS_LDADD)

object_notation_test_SOURCES = tests/object_notation.test.c \
	$(UNIT_TEST_SOURCES)

object_notation_test_CFLAGS = $(TESTS_CFLAGS)
object_notation_test_LDADD = $(TESTS_LDADD)

color_scheme_test_SOURCES = tests/color_scheme.test.c \
	$(UNIT_TEST_SOURCES)

color_scheme_test_CFLAGS = $(TESTS_CFLAGS)
color_scheme_test_LDADD = $(TESTS_LDADD)

bitvector_test_SOURCES = tests/bitvector.test.c \
	$(UNIT_TEST_SOURCES)

bitvector_test_CFLAGS = $(TESTS_CFLAGS)
bitvector_test_LDADD = $(TESTS_LDADD)

filter_expression_test_SOURCES = tests/filter_expression.test.c \
	$(UNIT_TEST_SOURCES)

filter_expression_test_CFLAGS = $(TESTS_CFLAGS)
filter_expression_test_LDADD = $(TESTS_LDADD)

monotonic_index_test_SOURCES = tests/monotonic_index.test.c \
	$(UNIT_TEST_SOURCES)

monotonic_index_test_CFLAGS = $(TESTS_CFLAGS)
monotonic_index_test_LDADD = $(TESTS_LDADD)

state_index_test_SOURCES = tests/state_index.test.c \
	$(UNIT_TEST_SOURCES)

state_index_test_CFLAGS = $(TESTS_CFLAGS)
state_index_test_LDADD = $(TESTS_LDADD)

hierarchy_test_SOURCES = tests/hierarchy.test.c \
	$(UNIT_TEST_SOURCES)

hierarchy_test_LDADD = $(TESTS_LDADD)
hierarchy_test_CFLAGS = $(TESTS_CFLAGS)

typed_array_test_SOURCES = tests/typed_array.test.c \
	$(UNIT_TEST_SOURCES)

typed_array_test_LDADD = $(TESTS_LDADD)
typed_array_test_CFLAGS = $(TESTS_CFLAGS)

EXTRA_DIST = $(UNIT_TEST_EXTRA_DIST) \
	$(TEXINFO_EXTRA_DIST) \
	$(DISTRO_EXTRA_DIST)


UPSTREAM_VERSION=@PACKAGE_VERSION@

deb:
	$(MAKE) dist-gzip
	rm -rf tmp/packaging/debian
	$(MKDIR_P) tmp/packaging/debian
	mv aftermath-$(UPSTREAM_VERSION).tar.gz tmp/packaging/debian/aftermath_$(UPSTREAM_VERSION).orig.tar.gz
	cd tmp/packaging/debian ; tar xf aftermath_$(UPSTREAM_VERSION).orig.tar.gz
	cp -ar $(srcdir)/distros/debian/build/debian tmp/packaging/debian/aftermath-$(UPSTREAM_VERSION)
	cd tmp/packaging/debian/aftermath-$(UPSTREAM_VERSION) ; debuild -us -uc
