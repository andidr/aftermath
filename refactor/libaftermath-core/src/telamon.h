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

enum am_telamon_candidate_flags {
	AM_TELAMON_CANDIDATE_FLAG_INTERNAL_NODE = (1 << 0),
	AM_TELAMON_CANDIDATE_FLAG_ROLLOUT_NODE = (1 << 1),
	AM_TELAMON_CANDIDATE_FLAG_IMPLEMENTATION = (1 << 2),
	AM_TELAMON_CANDIDATE_FLAG_DEADEND = (1 << 3),
	AM_TELAMON_CANDIDATE_FLAG_PERFMODEL_BOUND_VALID = (1 << 4),
};

enum am_telamon_candidate_type {
	AM_TELAMON_CANDIDATE_UNKNOWN,
	AM_TELAMON_CANDIDATE_INTERNAL_NODE,
	AM_TELAMON_CANDIDATE_ROLLOUT_NODE,
	AM_TELAMON_CANDIDATE_IMPLEMENTATION_NODE,
	AM_TELAMON_CANDIDATE_INTERNAL_DEADEND,
	AM_TELAMON_CANDIDATE_ROLLOUT_DEADEND,
	AM_TELAMON_CANDIDATE_IMPLEMENTATION_DEADEND
};

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

void am_telamon_count_nodes_dfs_callback(const struct am_telamon_candidate* node,
					 size_t depth,
					 void* data);

/* Define an iterative depth-first search function to determine the number of
 * nodes of a tree rooted at a candidate */
AM_DECL_DFS_FUNCTION(_telamon_candidate_count_nodes,
		     const struct am_telamon_candidate,
		     size_t*,
		     am_telamon_candidate_parent,
		     am_telamon_candidate_nth_child,
		     am_telamon_candidate_is_last_child,
		     am_telamon_candidate_child_idx,
		     am_telamon_candidate_has_children,
		     am_telamon_count_nodes_dfs_callback)

/* Returns the number of nodes (including the root) of the candidate tree rooted
 * at n using an iterative method. */
static inline size_t
am_telamon_candidate_tree_count_nodes(const struct am_telamon_candidate* n)
{
	size_t num_nodes = 0;

	am_dfs_norec_telamon_candidate_count_nodes(n, 20, &num_nodes);

	return num_nodes;
}

/* Returns the classification of a candidate at time t (including t) */
static inline enum am_telamon_candidate_type
am_telamon_candidate_get_type(const struct am_telamon_candidate* c,
			      am_timestamp_t t)
{
	uint32_t flags = 0;
	int is_known = 0;
	am_timestamp_t tmax = 0;

	/* First determine kind: internal / rollout / implementation */
	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_ROLLOUT_NODE) {
		if(c->rollout_time <= t && c->rollout_time >= tmax) {
			flags = AM_TELAMON_CANDIDATE_FLAG_ROLLOUT_NODE;
			tmax = c->rollout_time;
			is_known = 1;
		}
	}

	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_IMPLEMENTATION) {
		/* Implementations use rollout time */
		if(c->rollout_time <= t && c->rollout_time >= tmax) {
			flags = AM_TELAMON_CANDIDATE_FLAG_IMPLEMENTATION;
			tmax = c->rollout_time;
			is_known = 1;
		}
	}

	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_INTERNAL_NODE) {
		if(c->exploration_time <= t && c->exploration_time >= tmax) {
			flags = AM_TELAMON_CANDIDATE_FLAG_INTERNAL_NODE;
			tmax = c->exploration_time;
			is_known = 1;
		}
	}

	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_INTERNAL_NODE) {
		if(c->exploration_time <= t && c->exploration_time >= tmax) {
			flags = AM_TELAMON_CANDIDATE_FLAG_INTERNAL_NODE;
			tmax = c->exploration_time;
			is_known = 1;
		}
	}

	/* Then check if this is also a deadend */
	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_DEADEND) {
		if(c->deadend_time <= t) {
			flags |= AM_TELAMON_CANDIDATE_FLAG_DEADEND;
			is_known = 1;
		}
	}

	if(!is_known)
		return AM_TELAMON_CANDIDATE_UNKNOWN;

	if(flags & AM_TELAMON_CANDIDATE_FLAG_ROLLOUT_NODE) {
		if(flags & AM_TELAMON_CANDIDATE_FLAG_DEADEND)
			return AM_TELAMON_CANDIDATE_ROLLOUT_DEADEND;
		else
			return AM_TELAMON_CANDIDATE_ROLLOUT_NODE;
	} else if(flags & AM_TELAMON_CANDIDATE_FLAG_IMPLEMENTATION) {
		if(flags & AM_TELAMON_CANDIDATE_FLAG_DEADEND)
			return AM_TELAMON_CANDIDATE_IMPLEMENTATION_DEADEND;
		else
			return AM_TELAMON_CANDIDATE_IMPLEMENTATION_NODE;
	} else {
		if(flags & AM_TELAMON_CANDIDATE_FLAG_DEADEND)
			return AM_TELAMON_CANDIDATE_INTERNAL_DEADEND;
		else
			return AM_TELAMON_CANDIDATE_INTERNAL_NODE;
	}
}

/* Returns true if the value for the minimal bound of the performance model is
 * valid for c */
static inline int
am_telamon_candidate_perfmodel_bound_valid(const struct am_telamon_candidate* c)
{
	return c->flags & AM_TELAMON_CANDIDATE_FLAG_PERFMODEL_BOUND_VALID;
}

/* Returns the timestamp of the first encounter of a candidate */
static inline am_timestamp_t
am_telamon_candidate_first_encounter(const struct am_telamon_candidate* c)
{
	am_timestamp_t t = AM_TIMESTAMP_T_MAX;

	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_ROLLOUT_NODE) {
		if(c->rollout_time < t)
			t = c->rollout_time;
	}

	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_INTERNAL_NODE) {
		if(c->exploration_time < t)
			t = c->exploration_time;
	}

	if(c->flags & AM_TELAMON_CANDIDATE_FLAG_DEADEND) {
		if(c->deadend_time < t)
			t = c->deadend_time;
	}

	return t;
}

#endif
