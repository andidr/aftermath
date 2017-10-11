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

#include "io_hierarchy_context.h"

void am_io_hierarchy_context_tree_destroy(struct am_io_hierarchy_context_tree* hct)
{
	struct am_io_hierarchy_context_tree_node* n;
	struct am_io_hierarchy_context_tree_node* next;

	n = am_io_hierarchy_context_tree_first(hct);

	while(n) {
		next = am_io_hierarchy_context_tree_next_postorder(n);
		free(n);
		n = next;
	}
}

void am_io_hierarchy_context_init(struct am_io_hierarchy_context* ctx)
{
	am_io_hierarchy_context_tree_array_init(&ctx->hierarchies);
}

void am_io_hierarchy_context_destroy(struct am_io_hierarchy_context* ctx)
{
	for(size_t i = 0; i < ctx->hierarchies.num_elements; i++)
		am_io_hierarchy_context_tree_destroy(&ctx->hierarchies.elements[i]);

	am_io_hierarchy_context_tree_array_destroy(&ctx->hierarchies);
}

/* Associates an hierarchy h with an id in a I/O hierarchy context. Returns 0 on
 * success, otherwise 1.
 */
int am_io_hierarchy_context_add_hierarchy(struct am_io_hierarchy_context* hc,
					  struct am_hierarchy* h,
					  am_hierarchy_id_t id)
{
	struct am_io_hierarchy_context_tree* ht;

	ht = am_io_hierarchy_context_tree_array_reserve_sorted(&hc->hierarchies, id);

	if(!ht)
		return 1;

	ht->id = id;
	ht->hierarchy = h;
	am_io_hierarchy_context_tree_init(ht);

	return 0;
}

/* Associates an hierarchy node hn with a hierarchy id and a node id in a I/O
 * hierarchy context. Returns 0 on success, otherwise 1.
 */
int am_io_hierarchy_context_add_hierarchy_node(struct am_io_hierarchy_context* hc,
					       am_hierarchy_id_t h_id,
					       struct am_hierarchy_node* hn,
					       am_hierarchy_node_id_t hn_id)
{
	struct am_io_hierarchy_context_tree* ht;
	struct am_io_hierarchy_context_tree_node* htn;

	if(!(ht = am_io_hierarchy_context_tree_array_bsearch(&hc->hierarchies, h_id)))
		return 1;

	if(!(htn = malloc(sizeof(*htn))))
		return 1;

	htn->id = hn_id;
	htn->hierarchy_node = hn;
	am_io_hierarchy_context_tree_insert(ht, htn);

	return 0;
}
