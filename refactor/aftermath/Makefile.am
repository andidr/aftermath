bin_PROGRAMS = aftermath
aftermath_SOURCES = src/AftermathSession.cpp \
	src/AftermathSession.h \
	src/AftermathController.cpp \
	src/AftermathController.h \
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
	src/dfg/nodes/gui/hierarchy_combobox.h \
	src/dfg/nodes/gui/hierarchy_combobox.cpp \
	src/dfg/nodes/gui/histogram.h \
	src/dfg/nodes/gui/histogram.cpp \
	src/dfg/nodes/gui/label.h \
	src/dfg/nodes/gui/label.cpp \
	src/dfg/nodes/gui/telamon_candidate_tree.h \
	src/dfg/nodes/gui/telamon_candidate_tree.cpp \
	src/dfg/nodes/gui/timeline.h \
	src/dfg/nodes/gui/timeline.cpp \
	src/dfg/nodes/gui/toolbar_button.h \
	src/dfg/nodes/gui/toolbar_button.cpp \
	src/dfg/nodes/gui/toolbar_togglebutton.h \
	src/dfg/nodes/gui/toolbar_togglebutton.cpp \
	src/dfg/types/builtin_types.h \
	src/dfg/types/builtin_types.c \
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
	src/gui/factory/HierarchyComboBoxCreator.h \
	src/gui/factory/HierarchyComboBoxCreator.cpp \
	src/gui/factory/HistogramWidgetCreator.cpp \
	src/gui/factory/HistogramWidgetCreator.h \
	src/gui/factory/LabelWidgetCreator.cpp \
	src/gui/factory/LabelWidgetCreator.h \
	src/gui/factory/SplitterWidgetCreator.h \
	src/gui/factory/StatusbarWidgetCreator.h \
	src/gui/factory/TabWidgetCreator.cpp \
	src/gui/factory/TabWidgetCreator.h \
	src/gui/factory/TelamonCandidateTreeWidgetCreator.cpp \
	src/gui/factory/TelamonCandidateTreeWidgetCreator.h \
	src/gui/factory/TimelineWidgetCreator.cpp \
	src/gui/factory/TimelineWidgetCreator.h \
	src/gui/factory/ToolbarButtonCreator.h \
	src/gui/factory/ToolbarWidgetCreator.h \
	src/models/DFGNodeTypeRegistryTableModel.cpp \
	src/models/moc_DFGNodeTypeRegistryTableModel.cpp \
	src/models/DFGNodeTypeRegistryTableModel.h \
	src/models/GUITreeItem.cpp \
	src/models/GUITreeItem.h \
	src/models/GUITreeModel.cpp \
	src/models/moc_GUITreeModel.cpp \
	src/models/GUITreeModel.h \
	src/gui/widgets/CairoWidget.cpp \
	src/gui/widgets/CairoWidget.h \
	src/gui/widgets/CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/moc_CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/CairoWidgetWithDFGNode.h \
	src/gui/widgets/DFGWidget.cpp \
	src/gui/widgets/moc_DFGWidget.cpp \
	src/gui/widgets/DFGWidget.h \
	src/gui/widgets/HierarchyComboBox.cpp \
	src/gui/widgets/moc_HierarchyComboBox.cpp \
	src/gui/widgets/HierarchyComboBox.h \
	src/gui/widgets/HistogramWidget.cpp \
	src/gui/widgets/HistogramWidget.h \
	src/gui/widgets/KDTreeWidget.cpp \
	src/gui/widgets/moc_KDTreeWidget.cpp \
	src/gui/widgets/KDTreeWidget.h \
	src/gui/widgets/ManagedWidget.h \
	src/gui/widgets/RectTreeWidget.cpp \
	src/gui/widgets/moc_RectTreeWidget.cpp \
	src/gui/widgets/RectTreeWidget.h \
	src/gui/widgets/TelamonCandidateTreeWidget.cpp \
	src/gui/widgets/TelamonCandidateTreeWidget.h \
	src/gui/widgets/TimelineWidget.cpp \
	src/gui/widgets/TimelineWidget.h \
	src/gui/widgets/ToolbarButton.cpp \
	src/gui/widgets/moc_ToolbarButton.cpp \
	src/gui/widgets/ToolbarButton.h \
	src/gui/widgets/WidgetWithDFGNode.h

share/profiles/min/interface.amgui: $(srcdir)/share/profiles/min/interface.amgui.in
	$(MKDIR_P) share/profiles/min/
	sed 's|@ICON_BASEDIR@|$(datarootdir)/aftermath/icons|g' $< > $@

share/profiles/telamon/interface.amgui: $(srcdir)/share/profiles/telamon/interface.amgui.in
	$(MKDIR_P) share/profiles/telamon/
	sed 's|@ICON_BASEDIR@|$(datarootdir)/aftermath/icons|g' $< > $@

share/profiles/telamon-candidate-stats/interface.amgui: $(srcdir)/share/profiles/telamon-candidate-stats/interface.amgui.in
	$(MKDIR_P) share/profiles/telamon-candidate-stats/
	sed 's|@ICON_BASEDIR@|$(datarootdir)/aftermath/icons|g' $< > $@

share/profiles/telamon-min/interface.amgui: $(srcdir)/share/profiles/telamon-min/interface.amgui.in
	$(MKDIR_P) share/profiles/telamon-min/
	sed 's|@ICON_BASEDIR@|$(datarootdir)/aftermath/icons|g' $< > $@

share/profiles/tensorflow/interface.amgui: $(srcdir)/share/profiles/tensorflow/interface.amgui.in
	$(MKDIR_P) share/profiles/tensorflow/
	sed 's|@ICON_BASEDIR@|$(datarootdir)/aftermath/icons|g' $< > $@

GENERATED_PROFILE_TEMPLATES = \
	$(srcdir)/share/profiles/min/interface.amgui.in \
	$(srcdir)/share/profiles/telamon/interface.amgui.in \
	$(srcdir)/share/profiles/telamon-candidate-stats/interface.amgui.in \
	$(srcdir)/share/profiles/telamon-min/interface.amgui.in \
	$(srcdir)/share/profiles/tensorflow/interface.amgui.in

GENERATED_PROFILE_FILES = \
	share/profiles/min/interface.amgui \
	share/profiles/telamon/interface.amgui \
	share/profiles/telamon-candidate-stats/interface.amgui \
	share/profiles/telamon-min/interface.amgui \
	share/profiles/tensorflow/interface.amgui

STATIC_PROFILE_FILES = \
	share/profiles/min/graph.dfg \
	share/profiles/telamon/graph.dfg \
	share/profiles/telamon-candidate-stats/graph.dfg \
	share/profiles/telamon-min/graph.dfg \
	share/profiles/tensorflow/graph.dfg

ICONS = \
	share/icons/draw_states.svg \
	share/icons/draw_tf_node_executions.svg

# Install profiles
install-data-hook: $(PROFILES) $(ICONS) $(GENERATED_PROFILE_FILES)
	for pf in $(STATIC_PROFILE_FILES); \
	do \
		pfbase=`echo $$pf | sed 's|^share/||g'` || exit $$?; \
		$(MKDIR_P) `dirname "$(DESTDIR)/$(datarootdir)/aftermath/$$pfbase"` || exit $$? ; \
		$(INSTALL_DATA) "$(srcdir)/$$pf" "$(DESTDIR)/$(datarootdir)/aftermath/$$pfbase" || exit $$? ; \
	done

	for pf in $(GENERATED_PROFILE_FILES); \
	do \
		pfbase=`echo $$pf | sed 's|^share/||g'` || exit $$?; \
		$(MKDIR_P) `dirname "$(DESTDIR)/$(datarootdir)/aftermath/$$pfbase"` || exit $$? ; \
		$(INSTALL_DATA) "$(builddir)/$$pf" "$(DESTDIR)/$(datarootdir)/aftermath/$$pfbase" || exit $$? ; \
	done

	cd "$(DESTDIR)/$(datarootdir)/aftermath/profiles" && ln -sfr min default

	for icon in $(ICONS); \
	do \
		iconbase=`echo $$icon | sed 's|^share/||g'` || exit $$?; \
		$(MKDIR_P) `dirname "$(DESTDIR)/$(datarootdir)/aftermath/$$iconbase"` || exit $$?; \
		$(INSTALL_DATA) "$(srcdir)/$$icon" "$(DESTDIR)/$(datarootdir)/aftermath/$$iconbase" || exit $$? ; \
	done

MOCFLAGS=-I$(srcdir)/src -I$(builddir)/src @AFTERMATH_CORE_INCLUDES@ @AFTERMATH_RENDER_INCLUDES@
aftermath_CXXFLAGS=$(MOCFLAGS) @CAIRO_CFLAGS@ -DAM_PROFILE_BASEDIR="\"$(datarootdir)/aftermath/profiles\"" -Wall -Werror

aftermath_LDADD=@AFTERMATH_RENDER_LIBS@ @AFTERMATH_CORE_LIBS@ @CAIRO_LIBS@

GENERATED_FILES=src/ui_MainWindow.h \
	src/moc_MainWindow.cpp \
	src/gui/widgets/moc_CairoWidgetWithDFGNode.cpp \
	src/gui/widgets/moc_DFGWidget.cpp \
	src/gui/widgets/moc_HierarchyComboBox.cpp \
	src/gui/widgets/moc_KDTreeWidget.cpp \
	src/gui/widgets/moc_RectTreeWidget.cpp \
	src/gui/widgets/moc_ToolbarButton.cpp \
	src/gui/moc_DFGNodePropertyDialog.cpp \
	src/gui/moc_DFGNodeTypeSelectionDialog.cpp \
	src/dfg/moc_DFGQTProcessor.cpp \
	src/models/moc_DFGNodeTypeRegistryTableModel.cpp \
	src/models/moc_GUITreeModel.cpp \
	$(GENERATED_PROFILE_FILES)

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

src/gui/widgets/moc_KDTreeWidget.cpp: \
	src/gui/widgets/KDTreeWidget.cpp \
	src/gui/widgets/KDTreeWidget.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/widgets/KDTreeWidget.h -o $@

src/gui/widgets/moc_RectTreeWidget.cpp: \
	src/gui/widgets/RectTreeWidget.cpp \
	src/gui/widgets/RectTreeWidget.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/widgets/RectTreeWidget.h -o $@

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

src/models/moc_GUITreeModel.cpp: \
	src/models/GUITreeModel.cpp \
	src/models/GUITreeModel.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/models/GUITreeModel.h -o $@

src/gui/widgets/moc_HierarchyComboBox.cpp: \
	src/gui/widgets/HierarchyComboBox.cpp \
	src/gui/widgets/HierarchyComboBox.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/widgets/HierarchyComboBox.h -o $@

src/gui/widgets/moc_ToolbarButton.cpp: \
	src/gui/widgets/ToolbarButton.cpp \
	src/gui/widgets/ToolbarButton.h
	$(moc_verbose)$(MOC) $(MOCFLAGS) $(srcdir)/src/gui/widgets/ToolbarButton.h -o $@

BUILT_SOURCES = $(GENERATED_FILES)

clean-local:
	rm -f $(GENERATED_FILES)
	rm -rf tmp/packaging

EXTRA_DIST = src/MainWindow.ui \
	$(GENERATED_PROFILE_TEMPLATES) \
	$(STATIC_PROFILE_FILES) \
	$(ICONS)

UPSTREAM_VERSION=@PACKAGE_VERSION@

deb:
	$(MAKE) dist-gzip
	rm -rf tmp/packaging/debian
	$(MKDIR_P) tmp/packaging/debian
	mv aftermath-$(UPSTREAM_VERSION).tar.gz tmp/packaging/debian/aftermath_$(UPSTREAM_VERSION).orig.tar.gz
	cd tmp/packaging/debian ; tar xf aftermath_$(UPSTREAM_VERSION).orig.tar.gz
	cp -ar $(srcdir)/distros/debian/build/debian tmp/packaging/debian/aftermath-$(UPSTREAM_VERSION)
	cd tmp/packaging/debian/aftermath-$(UPSTREAM_VERSION) ; debuild -us -uc
