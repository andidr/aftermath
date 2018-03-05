nobase_include_HEADERS = aftermath/core/ansi_extras.h \
	aftermath/core/arithmetic.h \
	aftermath/core/aux.h \
	aftermath/core/base_types.h \
	aftermath/core/bitvector.h \
	aftermath/core/bsearch.h \
	aftermath/core/buffer.h \
	aftermath/core/contrib/linux-kernel/kernel.h \
	aftermath/core/contrib/linux-kernel/list.h \
	aftermath/core/contrib/linux-kernel/poison.h \
	aftermath/core/contrib/linux-kernel/rbtree_augmented.h \
	aftermath/core/contrib/linux-kernel/rbtree.h \
	aftermath/core/contrib/linux-kernel/stddef.h \
	aftermath/core/contrib/linux-kernel/types.h \
	aftermath/core/contrib/linux-kernel/userspace.h \
	aftermath/core/convert.h \
	aftermath/core/counter_description_array.h \
	aftermath/core/counter_event_array.h \
	aftermath/core/counter_event_array_collection.h \
	aftermath/core/default_event_array_registry.h \
	aftermath/core/dfg_builtin_node_impl.h \
	aftermath/core/dfg_builtin_node_types.h \
	aftermath/core/dfg_builtin_types.h \
	aftermath/core/dfg_buffer.h \
	aftermath/core/dfg_graph.h \
	aftermath/core/dfg_node.h \
	aftermath/core/dfg_node_type_registry.h \
	aftermath/core/dfg_schedule.h \
	aftermath/core/dfg_type.h \
	aftermath/core/dfg_type_registry.h \
	aftermath/core/dfg/nodes/interval_attributes.h \
	aftermath/core/event_mapping.h \
	aftermath/core/event_collection.h \
	aftermath/core/event_collection_array.h \
	aftermath/core/hierarchy.h \
	aftermath/core/hierarchy_array.h \
	aftermath/core/interval.h \
	aftermath/core/interval_array.h \
	aftermath/core/in_memory.h \
	aftermath/core/io_context.h \
	aftermath/core/io_error.h \
	aftermath/core/io_hierarchy_context.h \
	aftermath/core/measurement_interval_array.h \
	aftermath/core/object_notation.h \
	aftermath/core/on_disk.h \
	aftermath/core/parser.h \
	aftermath/core/ptr.h \
	aftermath/core/safe_alloc.h \
	aftermath/core/state_description_array.h \
	aftermath/core/state_event_array.h \
	aftermath/core/statistics/interval.h \
	aftermath/core/timestamp.h \
	aftermath/core/trace.h \
	aftermath/core/typed_array.h \
	aftermath/core/typed_list.h \
	aftermath/core/typed_rbtree.h

aftermath/core/base_types.h: $(top_builddir)/src/base_types.h
	$(MKDIR_P) aftermath/core
	ln -sfr $< $@

aftermath/core/on_disk.h: $(top_builddir)/src/on_disk.h
	$(MKDIR_P) aftermath/core
	ln -sfr $< $@

aftermath/core/in_memory.h: $(top_builddir)/src/in_memory.h
	$(MKDIR_P) aftermath/core
	ln -sfr $< $@
