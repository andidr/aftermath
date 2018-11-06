nobase_include_HEADERS = \
	aftermath/trace/base_types.h \
	aftermath/trace/buffered_event_collection.h \
	aftermath/trace/write_buffer.h

aftermath/trace/base_types.h: $(top_builddir)/src/base_types.h
	$(MKDIR_P) aftermath/trace
	ln -srf $< $@

aftermath/trace/write_buffer.h: $(top_builddir)/src/write_buffer.h
	$(MKDIR_P) aftermath/trace
	ln -srf $< $@
