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

#include <aftermath/core/indexes/kdtree.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/qselect.h>
#include <aftermath/core/ptr.h>

#define AM_KDTREE_NODE_PCMP(a, b) \
	AM_VALCMP_EXPR((*a)->coordinates[*data], (*b)->coordinates[*data])

AM_DECL_QSELECT_NTH_GREATEST_SUFFIX_DATA_ARG(
	am_kdtree_nodes_, _ptrs,
	struct am_kdtree_node*,
	AM_KDTREE_NODE_PCMP,
	size_t)

void am_kdtree_init(struct am_kdtree* t, size_t num_dimensions)
{
	t->num_dimensions = num_dimensions;
}

/* Recursively splits an array of k-d-tree node pointers on the median values
 * for the dimensions starting with dimension d.
 *
 * max_depth is the maximum allowed depth of recursive calls; If recursion
 * exceeds this limit, the function aborts.
 *
 * Returns 0 on success, otherwise 1 (e.g., if the maximum depth was exceeded).
 */
int am_kdtree_split(struct am_kdtree_node** out,
		    size_t dimension,
		    size_t num_dimensions,
		    struct am_kdtree_node** nodes,
		    size_t num_nodes,
		    size_t max_depth,
		    size_t curr_depth)
{
	size_t next_dimension = (dimension + 1) % num_dimensions;
	size_t median_idx;
	struct am_kdtree_node** median;

	if(curr_depth > max_depth)
		return 1;

	if(num_nodes == 0) {
		*out = NULL;
	} else if(num_nodes == 1) {
		*out = nodes[0];
		(*out)->smaller = NULL;
		(*out)->greater = NULL;
	} else if(num_nodes > 1) {
		/* Sort according to current dimension */
		if(!(median = am_kdtree_nodes_qselect_nth_greatest_ptrs(
			     nodes, num_nodes, num_nodes / 2, &dimension)))
		{
			return 1;
		}

		median_idx = AM_ARRAY_INDEX(nodes, median);
		*out = nodes[median_idx];

		/* Nodes with current dimension smaller */
		if(am_kdtree_split(&(*out)->smaller,
				   next_dimension,
				   num_dimensions,
				   &nodes[0],
				   median_idx,
				   max_depth,
				   curr_depth + 1))
		{
			return 1;
		}

		/* Nodes with current dimension greater */
		if(am_kdtree_split(&(*out)->greater,
				   next_dimension,
				   num_dimensions,
				   &nodes[median_idx+1],
				   (num_nodes - median_idx) - 1,
				   max_depth,
				   curr_depth + 1))
		{
			return 1;
		}
	}

	return 0;
}

/* Builds a k-d-tree from an array of pointers to k-d-tree nodes whose
 * coordinates have been set.
 *
 * max_depth is the maximum allowed depth of recursive calls; If recursion
 * exceeds this limit, the function aborts.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_kdtree_build(struct am_kdtree* t,
		    struct am_kdtree_node** nodes,
		    size_t num_nodes,
		    size_t max_depth)
{
	return am_kdtree_split(&t->root, 0, t->num_dimensions, nodes, num_nodes,
			       max_depth, 0);
}

/* Invokes the callback function cb for each node of the k-d-sub-tree rooted at
 * node r whose coordinates are within the query hyper-rectangle defined by
 * query_start and query_end. The current dimension must match the dimension
 * according to which the root splits the tree and the number of dimensions must
 * match the dimensionality of the nodes. The argument data is passed verbatim
 * to the callback function.
 *
 * If the callback function returns a value different from 0, tree traversal is
 * stopped. The return value is 0 if the callback function was invoked for all
 * nodes in the query hyper-rectangle, otherwise 1. */
int am_kdtree_query_callback_node(struct am_kdtree_node* n,
				  size_t curr_dimension,
				  size_t num_dimensions,
				  double* query_start,
				  double* query_end,
				  int (*cb)(struct am_kdtree_node* n, void* data),
				  void* data)
{
	size_t next_dimension = (curr_dimension + 1) % num_dimensions;
	int included = 1;

	if(!n)
		return 0;

	/* Test root node of sub-tree itself */
	for(size_t d = 0; d < num_dimensions; d++) {
		if(n->coordinates[d] < query_start[d] ||
		   n->coordinates[d] > query_end[d])
		{
			included = 0;
			break;
		}
	}

	if(included) {
		/* Stop if callback has indicated to stop */
		if(cb(n, data))
			return 1;
	}

	/* Only descend into smaller branch if value of current node for current
	 * dimension is not already smaller than query */
	if(n->coordinates[curr_dimension] >= query_start[curr_dimension]) {
		if(am_kdtree_query_callback_node(n->smaller, next_dimension,
						 num_dimensions,
						 query_start, query_end, cb,
						 data))
		{
			return 1;
		}
	}

	/* Only descend into greater branch if value of current node for current
	 * dimension is not already greater than query */
	if(n->coordinates[curr_dimension] <= query_end[curr_dimension]) {
		if(am_kdtree_query_callback_node(n->greater, next_dimension,
						 num_dimensions,
						 query_start, query_end, cb,
						 data))
		{
			return 1;
		}
	}

	return 0;
}

/* Invokes the callback function cb for each node of the k-d-tree t whose
 * coordinates are within the query hyper-rectangle defined by query_start and
 * query_end. The number of values for query_start and query_end must match the
 * dimensionality of t. The argument data is passed verbatim to the callback
 * function.
 *
 * If the callback function returns a value different from 0, tree traversal is
 * stopped. The return value is 0 if the callback function was invoked for all
 * nodes in the query hyper-rectangle, otherwise 1. */
int am_kdtree_query_callback(const struct am_kdtree* t,
			     double* query_start,
			     double* query_end,
			     int (*cb)(struct am_kdtree_node* n, void* data),
			     void* data)
{
	return am_kdtree_query_callback_node(t->root, 0, t->num_dimensions,
					     query_start, query_end, cb, data);
}

/* Returns the depth of a subtree rooted at n (starting with n at depth 1). */
size_t am_kdtree_node_depth(const struct am_kdtree_node* n)
{
	size_t smaller_depth = 0;
	size_t greater_depth = 0;

	if(!n)
		return 0;

	if(n->smaller)
		smaller_depth = am_kdtree_node_depth(n->smaller);

	if(n->greater)
		greater_depth = am_kdtree_node_depth(n->greater);

	if(smaller_depth > greater_depth)
		return smaller_depth + 1;
	else
		return greater_depth + 1;
}

/* Returns the depth of a k-d-tree (starting with the root at depth 1). */
size_t am_kdtree_depth(const struct am_kdtree* t)
{
	return am_kdtree_node_depth(t->root);
}
