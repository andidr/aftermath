/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_FRAME_TYPE_REGISTRY_H
#define AM_FRAME_TYPE_REGISTRY_H

#include <aftermath/core/typed_rbtree.h>

struct am_io_context;

#define AM_FRAME_REGISTRY_READ_FUN(f) \
	((int (*)(struct am_io_context*, void*))(f))

#define AM_FRAME_REGISTRY_DESTROY_FUN(f) ((void (*)(void*))(f))

#define AM_FRAME_REGISTRY_DUMP_FUN(f) \
	((int (*)(struct am_io_context*, void*, size_t, size_t))(f))

/* Definition of a frame type */
struct am_frame_type {
	/* The name of the frame type */
	char* name;

	/* The sequential ID allocated to the frame type */
	size_t seq_id;

	/* Size in bytes of an unitialized structure that can hold a frame of
	 * this type */
	size_t size;

	/* Red-black tree node for lookup by name */
	struct rb_node node;

	/* Pointer to a function that reads the frame from disk and processes
	 * it */
	int (*load)(struct am_io_context* ctx);

	/* Pointer to a function that reads the frame from the disk at the
	 * current position of the I/O context, using an empty, not initialized
	 * structure pointed to by frame. */
	int (*read)(struct am_io_context* ctx, void* frame);

	/* Destroys a fully initialized instance of a frame */
	void (*destroy)(void* frame);

	/* Pointer to a function that dumps a frame to stdout. The indent
	 * parameter indicates by how many tabs the first line should be
	 * indented, while next_indent indicates by how many tabs all following
	 * lines whould be indented. */
	int (*dump_stdout)(struct am_io_context* ctx, void* frame, size_t indent,
			   size_t next_indent);
};

/* Red-black-tree for lookup of frame types by name */
struct am_frame_type_tree {
	struct rb_root entries_name;
};

/* Registry for frame types that allow for both lookup by name and by ID
 * alloctaed to a frame type */
struct am_frame_type_registry {
	/* Indexed by the frame type name */
	struct am_frame_type_tree name_tree;

	/* Indexed by sequential ID */
	struct am_frame_type** types;

	/* Maximum number of frame types the registry can hold */
	size_t max_types;
};

/* Returns the frame type associated with the ID seq_id or NULL if no such ID
 * has been registered. */
static inline struct am_frame_type*
am_frame_type_registry_by_id(struct am_frame_type_registry* r, size_t seq_id)
{
	if(seq_id > r->max_types)
		return NULL;

	return r->types[seq_id];
}

int am_frame_type_registry_init(struct am_frame_type_registry* r,
				size_t max_types);
void am_frame_type_registry_destroy(struct am_frame_type_registry* r);

struct am_frame_type*
am_frame_type_registry_find(struct am_frame_type_registry* r,
			    const char* type_name);

int am_frame_type_registry_add(struct am_frame_type_registry* r,
			       const char* type_name,
			       size_t size,
			       int (*load)(struct am_io_context* ctx),
			       int (*read)(struct am_io_context* ctx, void* frame),
			       void (*destroy)(void* frame),
			       int (*dump_stdout)(struct am_io_context* ctx,
						  void* frame,
						  size_t indent,
						  size_t next_indent));

int am_frame_type_registry_set_id(struct am_frame_type_registry* r,
				  struct am_frame_type* ft,
				  size_t seq_id);

#endif
