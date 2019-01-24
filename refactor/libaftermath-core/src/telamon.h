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

#ifndef AM_TELAMON_H
#define AM_TELAMON_H

#include <aftermath/core/dfs.h>
#include <aftermath/core/in_memory.h>
#include <stdlib.h>

/* Returns the parent of a candidate c. */
static inline struct am_telamon_candidate*
am_telamon_candidate_parent(const struct am_telamon_candidate* c)
{
	return c->parent;
}

/* Returns a pointer to the n-th child of c. The index n must be smaller than
 * the number of children of c. */
static inline struct am_telamon_candidate*
am_telamon_candidate_nth_child(const struct am_telamon_candidate* c, size_t n)
{
	return c->children[n];
}

/* Returns true if c has at least one child, otherwise false. */
static inline int
am_telamon_candidate_has_children(const struct am_telamon_candidate* c)
{
	return c->num_children > 0;
}

/* Returns true if child is the last child of c, otherwise false. */
static inline int
am_telamon_candidate_is_last_child(const struct am_telamon_candidate* c,
				   const struct am_telamon_candidate* child)
{
	return am_telamon_candidate_has_children(c) &&
		c->children[c->num_children-1] == child;
}

/* Returns the index of a child in the list of children of a candidate c. The
 * child node must be a child of c, otherwise the function returns SIZE_MAX. */
static inline
size_t am_telamon_candidate_child_idx(const struct am_telamon_candidate* c,
				      const struct am_telamon_candidate* child)
{
	for(size_t i = 0; i < c->num_children; i++)
		if(c->children[i] == child)
			return i;

	return SIZE_MAX;
}

/* Internal callback function for am_dfs_norec_telamon_candidate_depth */
void am_telamon_depth_dfs_callback(const struct am_telamon_candidate* node,
				   size_t depth,
				   void* data);

/* Define an iterative depth-first search function to determine the depth of a
 * tree rooted at a candidate */
AM_DECL_DFS_FUNCTION(_telamon_candidate_depth,
		     const struct am_telamon_candidate,
		     size_t*,
		     am_telamon_candidate_parent,
		     am_telamon_candidate_nth_child,
		     am_telamon_candidate_is_last_child,
		     am_telamon_candidate_child_idx,
		     am_telamon_candidate_has_children,
		     am_telamon_depth_dfs_callback)

/* Returns the depth of the candidate tree rooted at n using an iterative
 * method. The root itself is at depth 1. */
static inline size_t
am_telamon_candidate_tree_depth(const struct am_telamon_candidate* n)
{
	size_t depth = 0;

	am_dfs_norec_telamon_candidate_depth(n, 20, &depth);

	return depth + 1;
}

#endif
