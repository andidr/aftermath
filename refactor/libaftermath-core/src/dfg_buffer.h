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

#ifndef AM_DFG_BUFFER_H
#define AM_DFG_BUFFER_H

#include <aftermath/core/dfg_type.h>

/*
 * Buffer for data exchanges between nodes
 */
struct am_dfg_buffer {
	/* Type of the data */
	const struct am_dfg_type* sample_type;

	/* Number of samples currently present in the buffer */
	size_t num_samples;

	/* maixmum number of samples that this buffer can hold without
	 * resizing */
	size_t max_samples;

	/* Pointer to the actual sample data */
	void* data;

	/* List for all buffers (e.g., of a graph) */
	struct list_head list;

	/* Number of references to this buffer */
	int num_refs;
};

void am_dfg_buffer_init(struct am_dfg_buffer* b,
			const struct am_dfg_type* sample_type);
void am_dfg_buffer_reset(struct am_dfg_buffer* b);
void am_dfg_buffer_change_type(struct am_dfg_buffer* b,
			       const struct am_dfg_type* sample_type);
void am_dfg_buffer_destroy(struct am_dfg_buffer* b);
void am_dfg_buffer_inc_ref(struct am_dfg_buffer* b);
void am_dfg_buffer_dec_ref(struct am_dfg_buffer* b);

int am_dfg_buffer_resize(struct am_dfg_buffer* b, size_t num_samples);
int am_dfg_buffer_write(struct am_dfg_buffer* b, size_t num_samples, void* data);
void* am_dfg_buffer_reserve(struct am_dfg_buffer* b, size_t num_samples);
int am_dfg_buffer_shrink(struct am_dfg_buffer* b, size_t num_samples);
int am_dfg_buffer_read(struct am_dfg_buffer* b, size_t num_samples, void* data);
int am_dfg_buffer_read_last(struct am_dfg_buffer* b, void* data);
int am_dfg_buffer_get(struct am_dfg_buffer* b,
		   size_t sample_offset,
		   size_t num_samples,
		   void* data);
int am_dfg_buffer_ptr(struct am_dfg_buffer* b,
		      size_t sample_offset,
		      size_t num_sample_ptrs,
		      void** ptrs);
int am_dfg_buffer_last_ptr(struct am_dfg_buffer* b, void** ptrs);

#endif
