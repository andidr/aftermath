ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libaftermath-render.la

AM_CPPFLAGS = -I$(srcdir)/headers -I$(builddir)/headers

libaftermath_render_la_SOURCES = src/cairo_extras.c \
	src/cairo_extras.h \
	src/color_map.h \
	src/dfg/dfg_coordinate_mapping.h \
	src/dfg/dfg_coordinate_mapping.c \
	src/dfg/nodes/builtin_nodes.c \
	src/dfg/nodes/builtin_nodes.h \
	src/dfg/nodes/rgba_constant.c \
	src/dfg/nodes/rgba_constant.h \
	src/dfg/nodes/timeline/layers/axes.c \
	src/dfg/nodes/timeline/layers/axes.h \
	src/dfg/nodes/timeline/layers/background.c \
	src/dfg/nodes/timeline/layers/background.h \
	src/dfg/nodes/timeline/layers/hierarchy.c \
	src/dfg/nodes/timeline/layers/hierarchy.h \
	src/dfg/nodes/timeline/layers/state.c \
	src/dfg/nodes/timeline/layers/state.h \
	src/dfg/nodes/timeline/layers/tensorflow_node_execution.c \
	src/dfg/nodes/timeline/layers/tensorflow_node_execution.h \
	src/dfg/renderer.c \
	src/dfg/renderer.h \
	src/dfg/timeline_layer_common.h \
	src/dfg/types/builtin_types.c \
	src/dfg/types/builtin_types.h \
	src/dfg/types/rgba.c \
	src/dfg/types/rgba.h \
	src/histogram/renderer.c \
	src/histogram/renderer.h \
	src/kdtree/renderer.c \
	src/kdtree/renderer.h \
	src/recttree/renderer.c \
	src/recttree/renderer.h \
	src/telamon/candidate_tree_renderer.c \
	src/telamon/candidate_tree_renderer.h \
	src/timeline/common_layers.c \
	src/timeline/common_layers.h \
	src/timeline/layer.c \
	src/timeline/layer.h \
	src/timeline/layers/axes.c \
	src/timeline/layers/axes.h \
	src/timeline/layers/background.c \
	src/timeline/layers/background.h \
	src/timeline/layers/hierarchy.c \
	src/timeline/layers/hierarchy.h \
	src/timeline/layers/lane.c \
	src/timeline/layers/lane.h \
	src/timeline/layers/lane/state_event.c \
	src/timeline/layers/lane/state_event.h \
	src/timeline/layers/lane/openmp/openmp.c \
	src/timeline/layers/lane/openmp/openmp.h \
	src/timeline/layers/lane/tensorflow/node_execution.c \
	src/timeline/layers/lane/tensorflow/node_execution.h \
	src/timeline/layers/measurement_intervals.c \
	src/timeline/layers/measurement_intervals.h \
	src/timeline/layers/interval.c \
	src/timeline/layers/interval.h \
	src/timeline/layers/selection.c \
	src/timeline/layers/selection.h \
	src/timeline/layers/selection_array_defs.h \
	src/timeline/renderer.c \
	src/timeline/renderer.h \
	src/tree_layout.c \
	src/tree_layout.h

libaftermath_render_la_CFLAGS = @AFTERMATH_CORE_INCLUDES@ @CAIRO_CFLAGS@
libaftermath_render_la_LIBADD = @AFTERMATH_CORE_LIBS@ @CAIRO_LIBS@

libaftermath_renderdir = $(includedir)/aftermath/render
libaftermath_render_HEADERS =

clean-local:
	rm -rf tmp/packaging

SUBDIRS = headers
EXTRA_DIST = headers

UPSTREAM_VERSION=@PACKAGE_VERSION@

deb:
	$(MAKE) dist-gzip
	rm -rf tmp/packaging/debian
	$(MKDIR_P) tmp/packaging/debian
	mv libaftermath-render-$(UPSTREAM_VERSION).tar.gz tmp/packaging/debian/libaftermath-render_$(UPSTREAM_VERSION).orig.tar.gz
	cd tmp/packaging/debian ; tar xf libaftermath-render_$(UPSTREAM_VERSION).orig.tar.gz
	cp -ar $(srcdir)/distros/debian/build/debian tmp/packaging/debian/libaftermath-render-$(UPSTREAM_VERSION)
	cd tmp/packaging/debian/libaftermath-render-$(UPSTREAM_VERSION) ; debuild -us -uc
