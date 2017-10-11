nobase_include_HEADERS = aftermath/core/ansi_extras.h \
	aftermath/core/base_types.h \
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
	aftermath/core/io_error.h \
	aftermath/core/typed_array.h \
	aftermath/core/typed_list.h \
	aftermath/core/typed_rbtree.h

aftermath/core/base_types.h: $(top_builddir)/src/base_types.h
	$(MKDIR_P) aftermath/core
	ln -sfr $< $@
