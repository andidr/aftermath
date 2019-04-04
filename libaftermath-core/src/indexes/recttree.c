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

#include <aftermath/core/indexes/recttree.h>
#include <aftermath/core/safe_alloc.h>
#include <string.h>

void am_recttree_init(struct am_recttree* t, size_t num_dimensions)
{
	am_kdtree_init(&t->kdtree, num_dimensions);
}

/* Temporary data structure needed when updating the bounding box maximum values
 * for a tree node */
struct am_recttree_bbox_tmp {
	/* Bounding boxes ("lower right corners") at the current depth for all
	 * nodes up to the root */
	double* maxes;

	/* Number of dimensions for the hyper rectangles */
	size_t num_dimensions;
};

/* Initializes a temporary data structure used to determine the bounding box of
 * a tree node.
 *
 * Returns 0 on success, otherwise 1.
 */
static int am_recttree_bbox_tmp_init(struct am_recttree_bbox_tmp* bbtmp,
				     size_t num_dimensions,
				     size_t max_depth)
{
	size_t num_bytes;

	if(am_size_mul_safe(&num_bytes, num_dimensions, sizeof(double)))
		return 1;

	if(am_size_mul_safe(&num_bytes, num_bytes, max_depth))
		return 1;

	if(!(bbtmp->maxes = malloc(num_bytes)))
		return 1;

	bbtmp->num_dimensions = num_dimensions;

	return 0;
}

/* Destroys a temporary data structure used to determine the bounding box of a
 * tree node. */
void am_recttree_bbox_tmp_destroy(struct am_recttree_bbox_tmp* bbtmp)
{
	free(bbtmp->maxes);
}

/* Extends the maximum coordinates of a bounding box in out with the values from
 * m which are higher. */
void am_recttree_bbox_max_extend(size_t num_dimensions, double* out, double* m)
{
	for(size_t dim = 0; dim < num_dimensions; dim++)
		if(m[dim] > out[dim])
			out[dim] = m[dim];
}

/* Updates the bounding box for a tree node */
void am_recttree_update_bbox(struct am_recttree_node* n,
			     struct am_recttree_bbox_tmp* bbtmp,
			     size_t depth)
{
	struct am_recttree_node* smaller;
	struct am_recttree_node* greater;
	size_t this_dimension = depth % bbtmp->num_dimensions;
	double* this_bbox_max = &bbtmp->maxes[depth * bbtmp->num_dimensions];
	double* smaller_bbox_max;
	double* greater_bbox_max;

	/* Initialize with own hyper rectangle */
	memcpy(this_bbox_max,
	       n->hyperrect_end,
	       bbtmp->num_dimensions * sizeof(double));

	/* Extend for rectangles with smaller values for this dimension */
	if(n->kdnode.smaller) {
		smaller_bbox_max = this_bbox_max + bbtmp->num_dimensions;
		smaller = (struct am_recttree_node*)n->kdnode.smaller;
		am_recttree_update_bbox(smaller, bbtmp, depth + 1);
		am_recttree_bbox_max_extend(bbtmp->num_dimensions,
					    this_bbox_max,
					    smaller_bbox_max);
	}

	/* Extend for rectangles with greater values for this dimension */
	if(n->kdnode.greater) {
		greater_bbox_max = this_bbox_max + bbtmp->num_dimensions;
		greater = (struct am_recttree_node*)n->kdnode.greater;
		am_recttree_update_bbox(greater, bbtmp, depth + 1);
		am_recttree_bbox_max_extend(bbtmp->num_dimensions,
					    this_bbox_max,
					    greater_bbox_max);
	}

	/* Set maximum value for this node */
	n->bbox_max = this_bbox_max[this_dimension];
}

/* Builds a rect tree from a an array of pointers to tree nodes whose
 * coordinates have been set prior to the call.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_recttree_build(struct am_recttree* t,
		      struct am_recttree_node** nodes,
		      size_t num_nodes,
		      size_t max_depth)
{
	size_t num_dimensions = t->kdtree.num_dimensions;
	struct am_recttree_bbox_tmp bbtmp;
	size_t actual_depth;

	/* Passing the recttree pointers directly as k-d-tree node pointers
	 * works, since each recttree node has a k-d-tree node as its first
	 * member */
	if(am_kdtree_build(&t->kdtree, (struct am_kdtree_node**)nodes, num_nodes,
			   max_depth))
	{
		return 1;
	}

	actual_depth = am_kdtree_depth(&t->kdtree);

	/* No need to calculate bounding boxes for empty tree */
	if(actual_depth == 0)
		return 0;

	if(am_recttree_bbox_tmp_init(&bbtmp, num_dimensions, actual_depth))
		return 1;

	am_recttree_update_bbox((struct am_recttree_node*)t->kdtree.root,
				&bbtmp,
				0);

	am_recttree_bbox_tmp_destroy(&bbtmp);

	return 0;
}

/* Invokes the callback function cb for each node of the subtree rooted at n
 * whose hyper rectangle overlaps with the query hyper rectangle defined by
 * query_start and query_end. If the callback function returns a value different
 * from 0, no further invocations of the callback function take place.
 *
 * Returns 0 if the callback function was invoked for all overlapping hyper
 * rectangles or 1 if enumeration has been interrupted by the callback function.
 */
int am_recttree_query_callback_node(struct am_recttree_node* n,
				    size_t curr_dimension,
				    size_t num_dimensions,
				    const double* query_start,
				    const double* query_end,
				    int (*cb)(struct am_recttree_node* n, void* data),
				    void* data)
{
	size_t next_dimension = (curr_dimension + 1) % num_dimensions;

	if(!n)
		return 0;

	/* Test root node of sub-tree itself */
	if(am_hyperrectangle_intersect_p(
		   num_dimensions,
		   query_start, query_end,
		   n->kdnode.coordinates, n->hyperrect_end))
	{
		/* Stop if callback has indicated to stop */
		if(cb(n, data))
			return 1;
	}

	/* Only descend into smaller branch if maximum coordinate for this
	 * dimension in subtree is not smaller than the value for the same
	 * dimension for the start of the query hyper rectangle:
	 *
	 *    +----- BBOX -----+  Bounding box for all rectangles with smaller
	 *    |                |  coordinates for the current dimension
	 *    |                |
	 *    |                |
	 *    |                |
	 *    +----------------+
	 *
	 *        QQQQQQQQQQQQQQQ  Query hyper rectangle
	 *        Q             Q
	 *   -----Q--+---- n ---Q--+------------- Separating hyper plane
	 *        Q  |          Q  |
	 *        Q  |          Q  |
	 *        QQQQQQQQQQQQQQQ  |
	 *           |             |  Hyper rectangle for n
	 *           +-------------+
	 */
	if(!(n->bbox_max < query_start[curr_dimension])) {
		if(am_recttree_query_callback_node(
			   (struct am_recttree_node*)n->kdnode.smaller,
			   next_dimension, num_dimensions,query_start, query_end, cb, data))
		{
			return 1;
		}
	}

	/* Only descend into greater branch if value for the current dimension
	 * of the start of the node's hyper rectangle is not already higher than
	 * the value for the same dimension of the end of the query hyper
	 * rectangle:
	 *
	 *
	 *        QQQQQQQQQQQQQQQ  Query hyper rectangle
	 *        Q             Q
	 *        Q             Q
	 *        Q             Q
	 *        Q             Q
	 *        QQQQQQQQQQQQQQQ
	 *
	 *   --------+---- n ------+------------- Separating hyper plane
	 *           |             |
	 *           |             |
	 *           |             |
	 *           |             |  Hyper rectangle for n
	 *           +-------------+
	 */
	if(!(n->kdnode.coordinates[curr_dimension] > query_end[curr_dimension])) {
		if(am_recttree_query_callback_node(
			   (struct am_recttree_node*)n->kdnode.greater,
			   next_dimension, num_dimensions,
			   query_start, query_end, cb,
			   data))
		{
			return 1;
		}
	}

	return 0;
}

/* Invokes the callback function cb for each node of the rect tree whose hyper
 * rectangle overlaps with the query hyper rectangle defined by query_start and
 * query_end. If the callback function returns a value different from 0, no
 * further invocations of the callback function take place.
 *
 * Returns 0 if the callback function was invoked for all overlapping hyper
 * rectangles or 1 if enumeration has been interrupted by the callback function.
 */
int am_recttree_query_callback(const struct am_recttree* t,
			       const double* query_start,
			       const double* query_end,
			       int (*cb)(struct am_recttree_node* n, void* data),
			       void* data)
{
	return am_recttree_query_callback_node(
		(struct am_recttree_node*)t->kdtree.root,
		0, t->kdtree.num_dimensions,
		query_start, query_end, cb, data);
}

static int am_recttree_node_at_callback(struct am_recttree_node* n,
					void* data)
{
	struct am_recttree_node** ret = data;
	*ret = n;

	/* Stop after first node */
	return 1;
}

/* Returns the first node whose bounding box overlaps with the coordinates
 * specified in p. The number of coordinates of p must match the dimensionality
 * of the rect tree.
 *
 * Returns NULL if no node overlaps with p.
 */
struct am_recttree_node* am_recttree_node_at(const struct am_recttree* t,
					     const double* pos)
{
	struct am_recttree_node* ret = NULL;

	am_recttree_query_callback(t, pos, pos, am_recttree_node_at_callback, &ret);

	return ret;
}
