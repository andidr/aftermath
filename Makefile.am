ACLOCAL_AMFLAGS=-I m4
VALGRIND=valgrind
bin_PROGRAMS = ostv
bin_SCRIPTS = ostv-debug-syms
TESTS =
noinst_PROGRAMS = $(TESTS)
LIB_CFLAGS = -D_POSIX_C_SOURCE=200112 -I$(srcdir)/src -Wall
TESTS_LDADD = -L. -lm -lpthread
TESTS_CFLAGS =  $(LIB_CFLAGS) -D_POSIX_C_SOURCE=200112 -I$(srcdir)/tests/lib -I$(srcdir)/src
UNIT_TEST_SOURCES = tests/lib/unit_tests.c tests/lib/unit_tests.h

ostv_SOURCES = src/ostv.c \
		src/trace_widget.c \
		src/buffer.h \
		src/buffer.c \
		src/events.h \
		src/events.c \
		src/filter.h \
		src/filter.c \
		src/paraver.h \
		src/paraver.c \
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
		src/detect.h \
		src/detect.c \
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
		src/cell_renderer_color_button.h \
		src/cell_renderer_color_button.c \
		src/glade_extras.h \
		src/ansi_extras.h \
		$(UNIT_TEST_SOURCES)
ostv_CFLAGS = $(TESTS_CFLAGS) @GTK_CFLAGS@ @GLADE_CFLAGS@ -DDATA_PATH="\"${datarootdir}/ostv\""
ostv_LDADD = $(TESTS_LDADD) @GTK_LIBS@ @GLADE_LIBS@
ostv_LDFLAGS = -export-dynamic
ostvdir=${datarootdir}/ostv
dist_ostv_DATA = share/ui/ostv.glade \
		share/ui/goto_dialog.glade \
		share/ui/about_dialog.glade \
		share/ui/settings.glade \
		share/ui/progress_dialog.glade \
		share/ui/color_dialog.glade \
		share/ui/derived_counter_dialog.glade \
		share/icons/draw_states.svg \
		share/icons/draw_comm.svg \
		share/icons/draw_steals.svg \
		share/icons/draw_pushes.svg \
		share/icons/draw_reads.svg \
		share/icons/draw_size.svg \
		share/icons/draw_events.svg \
		share/icons/draw_counters.svg

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

ostv-debug-syms: src/ostv-debug-syms.sh
	cp $< $@
	chmod +x $@
