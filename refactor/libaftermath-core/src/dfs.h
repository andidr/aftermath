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

#ifndef AM_DFS_H
#define AM_DFS_H

#include <aftermath/core/circular_buffer_size.h>

/* Defines a depth-first search travsersal function.
 *
 * SUFFIX is a suffix appendend to the function name.
 *
 * NODE_T is the type of graph nodes.
 *
 * DATA_T is the type of data passed as an argument, in turn passed verbatim to
 * the callback expression.
 *
 * PARENT(node) is an expression returning the parent of a node.
 *
 * NTH_CHILD(node, n) returns the n-th child of a node.
 *
 * IS_LAST_CHILD(parent, child) evaluates to true if child is the last child of
 * parent, otherwise it must evaluate to false.
 *
 * CHILD_IDX(parent, child) is an expression that, given the parentand a child,
 * evaluates to the zero-based index of a child in the list of children of a
 * parent.
 *
 * HAS_CHILDREN(node) evaluates to true if a node has at least one child,
 * otherwise it must evaluate to false.
 *
 * CALLBACK(node, depth, data) is an expression that is evaluated for each
 * node. The parameter depth indicates the zero-based depth of the node (i.e.,
 * the root is at depth 0) and data is the data pointer passed to the function.
 */
#define AM_DECL_DFS_FUNCTION(SUFFIX, NODE_T, DATA_T, PARENT, NTH_CHILD,	\
			     IS_LAST_CHILD, CHILD_IDX, HAS_CHILDREN, CALLBACK)	\
	static inline void am_dfs_norec##SUFFIX(				\
		NODE_T* root, size_t cb_size, DATA_T data)			\
	{									\
		size_t curr_depth = 1;						\
		NODE_T* curr_node = root;					\
		struct am_circular_buffer_size cb;				\
		size_t cb_entries[cb_size];					\
		size_t restore_idx;						\
										\
		am_circular_buffer_size_static_init(&cb, cb_size, cb_entries);	\
										\
		while(1) {							\
			CALLBACK(curr_node, curr_depth, data);			\
										\
			if(HAS_CHILDREN(curr_node)) {				\
				/* Descend one level*/				\
				curr_depth++;					\
				am_circular_buffer_size_push(&cb, 0);		\
				curr_node = NTH_CHILD(curr_node, 0);		\
										\
				continue;					\
			}							\
										\
		next_child_up:							\
			if(curr_node == root)					\
				break;						\
										\
			if(!IS_LAST_CHILD(PARENT(curr_node), curr_node)) {	\
				if(am_circular_buffer_size_pop(		\
					   &cb, &restore_idx)) {		\
					restore_idx = CHILD_IDX(		\
						curr_node->parent, curr_node);	\
				}						\
										\
				am_circular_buffer_size_push(			\
					&cb, restore_idx + 1);			\
										\
				curr_node = NTH_CHILD(PARENT(curr_node),	\
						      (restore_idx+1));	\
				continue;					\
			} else {						\
				curr_node = PARENT(curr_node);			\
				am_circular_buffer_size_pop(&cb, &restore_idx); \
				curr_depth--;					\
				goto next_child_up;				\
			}							\
		}								\
	}

#endif
