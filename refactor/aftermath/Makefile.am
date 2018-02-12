bin_PROGRAMS = aftermath
aftermath_SOURCES = src/main.cpp \
	src/moc_MainWindow.cpp \
	src/MainWindow.cpp \
	src/MainWindow.h \
	src/gui/widgets/CairoWidget.cpp \
	src/gui/widgets/CairoWidget.h \
	src/gui/widgets/DFGWidget.cpp \
	src/gui/widgets/DFGWidget.h \
	src/gui/widgets/TimelineWidget.cpp \
	src/gui/widgets/TimelineWidget.h

MOCFLAGS=-I$(srcdir)/src -I$(builddir)/src @AFTERMATH_CORE_INCLUDES@ @AFTERMATH_RENDER_INCLUDES@
aftermath_CXXFLAGS=$(MOCFLAGS) -fpermissive @CAIRO_CFLAGS@

aftermath_LDADD=@AFTERMATH_RENDER_LIBS@ @AFTERMATH_CORE_LIBS@ @CAIRO_LIBS@

GENERATED_FILES=src/ui_MainWindow.h \
	src/moc_MainWindow.cpp

moc_verbose = $(moc_verbose_@AM_V@)
moc_verbose_ = $(moc_verbose_@AM_DEFAULT_V@)
moc_verbose_0 = @echo "  MOC     " $@;

uic_verbose = $(uic_verbose_@AM_V@)
uic_verbose_ = $(uic_verbose_@AM_DEFAULT_V@)
uic_verbose_0 = @echo "  UIC     " $@;

src/MainWindow.cpp: src/ui_MainWindow.h

src/ui_MainWindow.h: src/MainWindow.ui
	$(uic_verbose)$(UIC) $< -o $@

src/moc_MainWindow.cpp: src/MainWindow.cpp
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/MainWindow.h -o $@

BUILT_SOURCES = $(GENERATED_FILES)

clean-local:
	rm -f $(GENERATED_FILES)
