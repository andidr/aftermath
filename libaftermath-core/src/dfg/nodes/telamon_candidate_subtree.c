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

#include <aftermath/core/dfg/nodes/telamon_candidate_subtree.h>
#include <aftermath/core/dfs.h>
#include <aftermath/core/in_memory.h>
#include <aftermath/core/telamon.h>

struct descendant_ctx {
	struct am_dfg_buffer* buffer;
	int error;
};

static inline void
write_descendants_callback(const struct am_telamon_candidate* node,
			   size_t depth,
			   struct descendant_ctx* ctx)
{
	if(am_dfg_buffer_write(ctx->buffer, 1, &node))
		ctx->error = 1;
}

/* Define an iterative depth-first search function that writes all descendants
 * of a candidate to an output buffer */
AM_DECL_DFS_FUNCTION(_output_descendants,
		     const struct am_telamon_candidate,
		     struct descendant_ctx*,
		     am_telamon_candidate_parent,
		     am_telamon_candidate_nth_child,
		     am_telamon_candidate_is_last_child,
		     am_telamon_candidate_child_idx,
		     am_telamon_candidate_has_children,
		     write_descendants_callback)

int am_dfg_telamon_candidate_subtree_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	struct am_telamon_candidate** in;
	struct descendant_ctx ctx;

	if(am_dfg_port_activated_and_has_data(pin) &&
	   am_dfg_port_activated(pout))
	{
		ctx.buffer = pout->buffer;
		in = pin->buffer->data;

		for(size_t i = 0; i < pin->buffer->num_samples; i++) {
			ctx.error = 0;

			am_dfs_norec_output_descendants(in[i], 100, &ctx);

			if(ctx.error)
				return 1;
		}
	}

	return 0;
}
