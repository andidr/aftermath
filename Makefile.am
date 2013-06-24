ACLOCAL_AMFLAGS=-I m4
VALGRIND=valgrind
bin_PROGRAMS = ostv
TESTS =
noinst_PROGRAMS = $(TESTS)
LIB_CFLAGS = -D_POSIX_C_SOURCE=200112 -I$(srcdir)/src
TESTS_LDADD = -L. -lm
TESTS_CFLAGS =  $(LIB_CFLAGS) -D_POSIX_C_SOURCE=200112 -I$(srcdir)/tests/lib -I$(srcdir)/src
UNIT_TEST_SOURCES = tests/lib/unit_tests.c tests/lib/unit_tests.h

ostv_SOURCES = src/ostv.c \
		src/trace_widget.c \
		src/events.h \
		src/events.c \
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
		$(UNIT_TEST_SOURCES)
ostv_CFLAGS = $(TESTS_CFLAGS) @GTK_CFLAGS@ @GLADE_CFLAGS@ -DDATA_PATH="\"${datarootdir}/ostv\""
ostv_LDADD = $(TESTS_LDADD) @GTK_LIBS@ @GLADE_LIBS@
ostv_LDFLAGS = -export-dynamic
ostvdir=${datarootdir}/ostv
dist_ostv_DATA = share/ui/ostv.glade \
		share/ui/goto_dialog.glade

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
