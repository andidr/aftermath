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

#ifndef AM_RECTTREE_H
#define AM_RECTTREE_H

#include <aftermath/core/indexes/kdtree.h>

/* A search tree for hyper rectangles. The "upper left" coordinate of the hyper
 * rectangle is stored in the k-d-tree node and the "lower right" coordinatye in
 * hyperrect_end. */
struct am_recttree_node {
	struct am_kdtree_node kdnode;
	double* hyperrect_end;
	double bbox_max;
};

struct am_recttree {
	struct am_kdtree kdtree;
};

void am_recttree_init(struct am_recttree* t, size_t num_dimensions);
int am_recttree_build(struct am_recttree* t,
		      struct am_recttree_node** nodes,
		      size_t num_nodes,
		      size_t max_depth);
int am_recttree_query_callback(const struct am_recttree* t,
			       const double* query_start,
			       const double* query_end,
			       int (*cb)(struct am_recttree_node* n, void* data),
			       void* data);

/* Returns 1 if two hyperrectangles A and B (defined by a_start and a_end, and
 * b_start and b_end, respectively) overlap. Otherwise, 0 is returned.
 */
static inline int am_hyperrectangle_intersect_p(
	size_t num_dimensions,
	const double* a_start,
	const double* a_end,
	const double* b_start,
	const double* b_end)
{
	for(size_t dim = 0; dim < num_dimensions; dim++)
		if(a_end[dim] < b_start[dim] || a_start[dim] > b_end[dim])
			return 0;

	return 1;
}

struct am_recttree_node* am_recttree_node_at(const struct am_recttree* t,
					     const double* pos);

#endif
