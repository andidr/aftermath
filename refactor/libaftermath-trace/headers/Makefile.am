nobase_include_HEADERS = \
	aftermath/trace/base_types.h \
	aftermath/trace/buffered_event_collection.h \
	aftermath/trace/on_disk_write_to_buffer.h \
	aftermath/trace/safe_alloc.h \
	aftermath/trace/write_buffer.h

aftermath/trace/base_types.h: $(top_builddir)/src/base_types.h
	$(MKDIR_P) aftermath/trace
	ln -srf $< $@

aftermath/trace/on_disk_write_to_buffer.h: $(top_builddir)/src/on_disk_write_to_buffer.h
	$(MKDIR_P) aftermath/trace
	ln -srf $< $@

aftermath/trace/safe_alloc.h: $(top_builddir)/src/safe_alloc.h
	$(MKDIR_P) aftermath/trace
	ln -srf $< $@

aftermath/trace/write_buffer.h: $(top_builddir)/src/write_buffer.h
	$(MKDIR_P) aftermath/trace
	ln -srf $< $@
