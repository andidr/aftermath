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

#ifndef AM_TREE_LAYOUT_H
#define AM_TREE_LAYOUT_H

#include <stdint.h>
#include <stdlib.h>

/* Defines a tree layout function that places each node of a tree in a 2D
 * space. Each tree node might be associated with a 2D rectangle representing
 * its shape. The tree layout algorithm will make sure that no nodes overlap.
 *
 * SUFFIX is the suffix of the function to be defined.
 *
 * NODE_TYPE is the data type of a node.
 *
 * ACC_Y is an expression returning the assigned y coordinate for a node, given
 * a pointer of type NODE_TYPE.
 *
 * SET_X and SET_Y are expressions that each take a pointer of type NODE_TYPE
 * and a double and respectively set the x and y coordinate of the node.
 *
 * ACC_WIDTH is an expression that returns the width of a node given a pointer
 * of type NODE_TYPE.
 *
 * ACC_HEIGHT is an expression that returns the height of a node given a pointer
 * of type NODE_TYPE.
 *
 * NUM_CHILDREN is an expression that returns the number of children of a node
 * given a pointer of type NODE_TYPE.
 *
 * NTH_CHILD is an expression that returns a pointer to the n-th child of a
 * node, given a pointer to the node and a size_t for n.
 */

#define AM_DECL_TREE_LAYOUT_FUN(						\
	SUFFIX, NODE_TYPE, ACC_X, ACC_Y, SET_X, SET_Y, ACC_WIDTH, ACC_HEIGHT,	\
	NUM_CHILDREN, NTH_CHILD)						\
	/* Places the nodes of a tree rooted at root; max_depth must be the	\
	 * maximum number of edges from the root to any node of the tree.	\
	 *									\
	 * xgap is the horizontal spacing between sibling nodes ygap is the	\
	 * vertical spacing between parent and child nodes			\
	 *									\
	 * Returns 0 on success, otherwise 1.					\
	 */									\
	int									\
	am_tree_layout_2d##SUFFIX(NODE_TYPE* root, size_t max_depth,		\
			  double xgap, double ygap)				\
	{									\
		size_t curr_depth = 0;						\
		NODE_TYPE* child_node;						\
		NODE_TYPE* base_node;						\
		NODE_TYPE* first_child;					\
		NODE_TYPE* last_child;						\
		double child_min_x;						\
		double child_max_x;						\
		double mid_x;							\
		double width;							\
		size_t num_children;						\
										\
		struct {							\
			double x;						\
			double y;						\
			size_t idx;						\
			NODE_TYPE* base_node;					\
		}* depthinfo = NULL;						\
										\
		if(max_depth == 0)						\
			return 0;						\
										\
		if(!(depthinfo = (typeof(depthinfo))				\
		     calloc(max_depth, sizeof(*depthinfo))))			\
		{								\
			return 1;						\
		}								\
										\
		depthinfo[curr_depth].base_node = root;			\
										\
		while(1) {							\
			/* The base node is the node whose children will be	\
			 * processed next */					\
			base_node = depthinfo[curr_depth].base_node;		\
										\
			/* Place base node */					\
			SET_X(base_node, depthinfo[curr_depth].x);		\
			SET_Y(base_node, depthinfo[curr_depth].y);		\
										\
			/* Place the first child at lest in the same row as the \
			 * base node */					\
			if(NUM_CHILDREN(base_node) > 0) {			\
				if(depthinfo[curr_depth].x >			\
				   depthinfo[curr_depth+1].x) {		\
					depthinfo[curr_depth+1].x =		\
						depthinfo[curr_depth].x;	\
				}						\
			}							\
										\
			/* Update position for next sibling node of base node */\
			depthinfo[curr_depth].x += ACC_WIDTH(base_node) + xgap; \
										\
		level_up:							\
			/* Done with all children of base node? */		\
			if(depthinfo[curr_depth].idx ==			\
			   NUM_CHILDREN(base_node)) {				\
				if(curr_depth == 0) {				\
					break;					\
				} else {					\
					depthinfo[curr_depth-1].x =		\
						depthinfo[curr_depth].x;	\
					curr_depth--;				\
					base_node =				\
						depthinfo[curr_depth].base_node;\
					num_children = NUM_CHILDREN(base_node); \
										\
					/* Basic placement done; Now position	\
					 * horizontally centered wrt the	\
					 * childrenparent node */		\
					if(num_children > 0) {			\
						first_child =			\
							NTH_CHILD(base_node, 0);\
						last_child =			\
							NTH_CHILD(base_node,	\
								  num_children-1); \
										\
						child_min_x = ACC_X(first_child);\
						child_max_x =			\
							ACC_X(last_child)+	\
							ACC_WIDTH(last_child); \
										\
						mid_x = (child_min_x + child_max_x) / 2.0; \
						width = ACC_WIDTH(base_node);	\
										\
						SET_X(base_node, mid_x - width / 2.0); \
					}					\
					goto level_up;				\
				}						\
			}							\
										\
			/* Select child and prepare next level */		\
			child_node = NTH_CHILD(base_node,			\
				depthinfo[curr_depth].idx);			\
			depthinfo[curr_depth+1].base_node = child_node;	\
			depthinfo[curr_depth+1].idx = 0;			\
			depthinfo[curr_depth+1].y = ACC_Y(base_node) +		\
				ACC_HEIGHT(base_node) + ygap;			\
										\
			/* Set index of current depth, such that when processing\
			 * of the child returns, the next child node is	\
			 * processed */					\
			depthinfo[curr_depth].idx++;				\
										\
			curr_depth++;						\
		}								\
										\
		free(depthinfo);						\
										\
		return 0;							\
	}

#endif
