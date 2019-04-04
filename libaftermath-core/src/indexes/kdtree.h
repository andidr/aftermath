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

#ifndef AM_KDTREE_H
#define AM_KDTREE_H

#include <stdlib.h>

/* Node of a k-d-tree */
struct am_kdtree_node {
	/* Sub-tree whose values for the node's split dimension are smaller */
	struct am_kdtree_node* smaller;

	/* Sub-tree whose values for the node's split dimension are greater */
	struct am_kdtree_node* greater;

	/* Coordinates of the node itself */
	double* coordinates;
};

/* Returns true if n is a leaf node */
static inline int am_kdtree_node_is_leaf(const struct am_kdtree_node* n)
{
	return !n->smaller && !n->greater;
}

/* An index for n-dimensional points */
struct am_kdtree {
	/* Number of dimensions of each point */
	size_t num_dimensions;

	/* Root node that splits the entire set of points */
	struct am_kdtree_node* root;
};

void am_kdtree_init(struct am_kdtree* t, size_t num_dimensions);
int am_kdtree_build(struct am_kdtree* t,
		    struct am_kdtree_node** nodes,
		    size_t num_nodes,
		    size_t max_depth);

int am_kdtree_query_callback(const struct am_kdtree* t,
			     double* query_start,
			     double* query_end,
			     int (*cb)(struct am_kdtree_node* n, void* data),
			     void* data);

size_t am_kdtree_depth(const struct am_kdtree* t);

#endif
