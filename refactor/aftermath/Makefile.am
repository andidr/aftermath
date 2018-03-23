bin_PROGRAMS = aftermath
aftermath_SOURCES = src/AftermathSession.cpp \
	src/AftermathSession.h \
	src/cxx_extras.h \
	src/cxx_interoperability.h \
	src/Exception.cpp \
	src/Exception.h \
	src/main.cpp \
	src/moc_MainWindow.cpp \
	src/MainWindow.cpp \
	src/MainWindow.h \
	src/dfg/DFGQTProcessor.cpp \
	src/dfg/moc_DFGQTProcessor.cpp \
	src/dfg/DFGQTProcessor.h \
	src/dfg/nodes/builtin_nodes.h \
	src/dfg/nodes/builtin_nodes.c \
	src/dfg/nodes/gui/label.h \
	src/dfg/nodes/gui/label.cpp \
	src/dfg/nodes/gui/timeline.h \
	src/dfg/nodes/gui/timeline.cpp \
	src/gui/AftermathGUI.cpp \
	src/gui/AftermathGUI.h \
	src/gui/DFGNodePropertyDialog.cpp \
	src/gui/moc_DFGNodePropertyDialog.cpp \
	src/gui/DFGNodePropertyDialog.h \
	src/gui/DFGNodeTypeSelectionDialog.cpp \
	src/gui/moc_DFGNodeTypeSelectionDialog.cpp \
	src/gui/DFGNodeTypeSelectionDialog.h \
	src/gui/factory/BoxWidgetCreator.h \
	src/gui/factory/ButtonWidgetCreator.cpp \
	src/gui/factory/ButtonWidgetCreator.h \
	src/gui/factory/DefaultGUIFactory.cpp \
	src/gui/factory/DefaultGUIFactory.h \
	src/gui/factory/DFGWidgetCreator.cpp \
	src/gui/factory/DFGWidgetCreator.h \
	src/gui/factory/GUIFactory.cpp \
	src/gui/factory/GUIFactory.h \
	src/gui/factory/HistogramWidgetCreator.cpp \
	src/gui/factory/HistogramWidgetCreator.h \
	src/gui/factory/LabelWidgetCreator.cpp \
	src/gui/factory/LabelWidgetCreator.h \
	src/gui/factory/SplitterWidgetCreator.h \
	src/gui/factory/TabWidgetCreator.cpp \
	src/gui/factory/TabWidgetCreator.h \
	src/gui/factory/TimelineWidgetCreator.cpp \
	src/gui/factory/TimelineWidgetCreator.h \
	src/gui/factory/ToolbarButtonCreator.h \
	src/gui/factory/ToolbarWidgetCreator.h \
	src/models/DFGNodeTypeRegistryTableModel.cpp \
	src/models/moc_DFGNodeTypeRegistryTableModel.cpp \
	src/models/DFGNodeTypeRegistryTableModel.h \
	src/gui/widgets/CairoWidget.cpp \
	src/gui/widgets/CairoWidget.h \
	src/gui/widgets/CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/moc_CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/CairoWidgetWithDFGNode.h \
	src/gui/widgets/DFGWidget.cpp \
	src/gui/widgets/moc_DFGWidget.cpp \
	src/gui/widgets/DFGWidget.h \
	src/gui/widgets/HistogramWidget.cpp \
	src/gui/widgets/HistogramWidget.h \
	src/gui/widgets/TimelineWidget.cpp \
	src/gui/widgets/TimelineWidget.h

MOCFLAGS=-I$(srcdir)/src -I$(builddir)/src @AFTERMATH_CORE_INCLUDES@ @AFTERMATH_RENDER_INCLUDES@
aftermath_CXXFLAGS=$(MOCFLAGS) @CAIRO_CFLAGS@ -Wall -Werror

aftermath_LDADD=@AFTERMATH_RENDER_LIBS@ @AFTERMATH_CORE_LIBS@ @CAIRO_LIBS@

GENERATED_FILES=src/ui_MainWindow.h \
	src/moc_MainWindow.cpp \
	src/gui/widgets/moc_CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/moc_DFGWidget.cpp \
	src/gui/moc_DFGNodePropertyDialog.cpp \
	src/gui/moc_DFGNodeTypeSelectionDialog.cpp \
	src/dfg/moc_DFGQTProcessor.cpp \
	src/models/moc_DFGNodeTypeRegistryTableModel.cpp

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

src/gui/widgets/moc_CairoWidgetWithDFGNode.cpp: \
	src/gui/widgets/CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/CairoWidgetWithDFGNode.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/widgets/CairoWidgetWithDFGNode.h -o $@

src/dfg/moc_DFGQTProcessor.cpp: \
	src/dfg/DFGQTProcessor.cpp \
	src/dfg/DFGQTProcessor.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/dfg/DFGQTProcessor.h -o $@

src/gui/widgets/moc_DFGWidget.cpp: \
	src/gui/widgets/DFGWidget.cpp \
	src/gui/widgets/DFGWidget.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/widgets/DFGWidget.h -o $@

src/gui/moc_DFGNodePropertyDialog.cpp: \
	src/gui/DFGNodePropertyDialog.cpp \
	src/gui/DFGNodePropertyDialog.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/DFGNodePropertyDialog.h -o $@

src/gui/moc_DFGNodeTypeSelectionDialog.cpp: \
	src/gui/DFGNodeTypeSelectionDialog.cpp \
	src/gui/DFGNodeTypeSelectionDialog.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/DFGNodeTypeSelectionDialog.h -o $@

src/models/moc_DFGNodeTypeRegistryTableModel.cpp: \
	src/models/DFGNodeTypeRegistryTableModel.cpp \
	src/models/DFGNodeTypeRegistryTableModel.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/models/DFGNodeTypeRegistryTableModel.h -o $@

BUILT_SOURCES = $(GENERATED_FILES)

clean-local:
	rm -f $(GENERATED_FILES)
