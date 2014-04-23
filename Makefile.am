ACLOCAL_AMFLAGS=-I m4
VALGRIND=valgrind
bin_PROGRAMS = aftermath
bin_SCRIPTS = aftermath-debug-syms
TESTS =
noinst_PROGRAMS = $(TESTS)
LIB_CFLAGS = -D_POSIX_C_SOURCE=200112 -I$(srcdir)/src -Wall
TESTS_LDADD = -L. -lm -lpthread
TESTS_CFLAGS =  $(LIB_CFLAGS) -D_POSIX_C_SOURCE=200112 -I$(srcdir)/tests/lib -I$(srcdir)/src
UNIT_TEST_SOURCES = tests/lib/unit_tests.c tests/lib/unit_tests.h

aftermath_SOURCES = src/aftermath.c \
		src/trace_widget.c \
		src/histogram_widget.h \
		src/histogram_widget.c \
		src/multi_histogram_widget.h \
		src/multi_histogram_widget.c \
		src/matrix_widget.h \
		src/matrix_widget.c \
		src/intensity_matrix.h \
		src/intensity_matrix.c \
		src/buffer.h \
		src/buffer.c \
		src/events.h \
		src/events.c \
		src/filter.h \
		src/filter.c \
		src/signals.h \
		src/signals.c \
		src/globals.h \
		src/globals.c \
		src/marshal.h \
		src/marshal.c \
		src/dialogs.h \
		src/dialogs.c \
		src/trace_file.h \
		src/trace_file.c \
		src/debug.h \
		src/debug.c \
		src/settings.h \
		src/settings.c \
		src/task_list.h \
		src/task_list.c \
		src/frame_list.h \
		src/frame_list.c \
		src/counter_list.h \
		src/counter_list.c \
		src/bitvector.h \
		src/bitvector.c \
		src/counter_description.h \
		src/counter_event_set.h \
		src/counter_event_set.c \
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
		src/statistics.h \
		src/statistics.c \
		src/derived_counters.h \
		src/derived_counters.c \
		src/color.h \
		src/color.c \
		src/page.h \
		src/page.c \
		src/cell_renderer_color_button.h \
		src/cell_renderer_color_button.c \
		src/glade_extras.h \
		src/ansi_extras.h \
		src/numa_node_list.h \
		src/numa_node_list.c \
		src/annotation.h \
		src/annotation.c \
		src/visuals_file.h \
		src/visuals_file.c \
		src/convert.h \
		src/convert.c \
		src/cpu_list.h \
		src/cpu_list.c \
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
		src/address_range_tree.h \
		src/address_range_tree.c \
		src/task_instance.h \
		src/task_instance.c \
		src/task_instance_rw_tree.h \
		src/task_instance_rw_tree.c \
		src/task_instance_tree.h \
		src/task_instance_tree.c \
		$(UNIT_TEST_SOURCES)
aftermath_CFLAGS = $(TESTS_CFLAGS) @GTK_CFLAGS@ @GLADE_CFLAGS@ -DDATA_PATH="\"${datarootdir}/aftermath\""
aftermath_LDADD = $(TESTS_LDADD) @GTK_LIBS@ @GLADE_LIBS@
aftermath_LDFLAGS = -export-dynamic
aftermathdir=${datarootdir}/aftermath
dist_aftermath_DATA = share/ui/aftermath.glade \
		share/ui/goto_dialog.glade \
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
		share/icons/draw_states.svg \
		share/icons/draw_steals.svg \
		share/icons/draw_pushes.svg \
		share/icons/draw_reads.svg \
		share/icons/draw_writes.svg \
		share/icons/draw_size.svg \
		share/icons/draw_events.svg \
		share/icons/draw_counters.svg \
		share/icons/draw_annotations.svg \
		share/icons/heatmap.svg \
		share/icons/typemap.svg \
		share/icons/numamap_reads.svg \
		share/icons/numamap_writes.svg \
		share/icons/heatmap_numa.svg \
		share/icons/fit_all_cpus.svg \
		share/icons/draw_measurement_intervals.svg

clean-local:
	-find . -iname "*~" | xargs rm

%-silent:
	@$(MAKE) $* | EGREP="$(EGREP)" CC="$(CC)" SED="$(SED)" AR="$(AR)" ./scripts/build_silently_filter.sh

valgrind-check: $(foreach TEST,$(TESTS),valgrind-check-$(TEST))

check-%: %
	@./$<

valgrind-check-raw-%: %
	@echo -n "[VALGRIND]"
	@G_SLICE=always-malloc G_DEBUG=gc-friendly $(VALGRIND) --gen-suppressions=all --track-origins=yes --leak-check=full --show-reachable=yes --leak-resolution=high --error-exitcode=1 --suppressions=./tests/valgrind-suppressions --run-libc-freeres=no ./$< 2> valgrind.$<.log ; \
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

src/marshal.h: src/marshal.in
	glib-genmarshal --header $< > $@

src/marshal.c: src/marshal.in
	glib-genmarshal --body $< > $@

src/trace_widget.c: src/marshal.h
src/trace_widget.h: src/marshal.h

aftermath-debug-syms: src/aftermath-debug-syms.sh
	cp $< $@
	chmod +x $@
