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

#ifndef AM_TYPED_RBTREE_H
#define AM_TYPED_RBTREE_H

#include "./contrib/linux-kernel/rbtree.h"

/* Defines a set of functions for a types red-black tree.
 *
 *   prefix: Prefix for all functions
 *   TREE_T: Data type for an entire tree (e.g., data struct containing the
 *           root)
 *   TREE_ROOTMEMB: Name of the rb_root element in TREE_T
 *   NODE_T: Data type for a tree node (e.g., struct containing rb_node)
 *   NODE_RBMEMB: Name of the rb_node element in NODE_T
 *   KEYACC_EXPR: Macro with an expression returning the value used as the key
 *                for a node.
 *   KEY_T: Type of the key
 */
#define AM_DECL_TYPED_RBTREE_OPS(PREFIX, TREE_T, TREE_ROOTMEMB, NODE_T,	\
				 NODE_RBMEMB, KEY_T, KEYACC_EXPR)		\
										\
	/* Initialize the tree */						\
	static inline void PREFIX##_init(TREE_T* t)				\
	{									\
		t->TREE_ROOTMEMB = RB_ROOT;					\
	}									\
										\
	/* Insert a node n into a tree t. Returns 0 on success, otherwise 1.*/	\
	static inline int PREFIX##_insert(TREE_T* t, NODE_T* n)		\
	{									\
		struct rb_node** new = &t->TREE_ROOTMEMB.rb_node;		\
		struct rb_node* parent = NULL;					\
										\
		/* Figure out where to put new node */				\
		while(*new) {							\
			NODE_T* this = container_of(*new, NODE_T, NODE_RBMEMB); \
			parent = *new;						\
										\
			if (KEYACC_EXPR(*n) < KEYACC_EXPR(*this))		\
				new = &((*new)->rb_left);			\
			else if (KEYACC_EXPR(*n) > KEYACC_EXPR(*this))		\
				new = &((*new)->rb_right);			\
			else							\
				return 1; /* Already in the tree */		\
		}								\
										\
		/* Add new node and rebalance tree. */				\
		rb_link_node(&n->NODE_RBMEMB, parent, new);			\
		rb_insert_color(&n->NODE_RBMEMB, &t->TREE_ROOTMEMB);		\
										\
		return 0;							\
	}									\
										\
	/* Remove a node n from a tree t. */					\
	static inline void PREFIX##_remove(TREE_T* t, NODE_T* n)		\
	{									\
		rb_erase(&n->NODE_RBMEMB, &t->TREE_ROOTMEMB);			\
	}									\
										\
	/* Return the first node (with the lowest key) or NULL if the tree is	\
	 * empty. */								\
	static inline NODE_T* PREFIX##_first(TREE_T* t)			\
	{									\
		struct rb_node* node = rb_first(&t->TREE_ROOTMEMB);		\
										\
		if(!node)							\
			return NULL;						\
										\
		NODE_T* this_node = rb_entry(node, NODE_T, NODE_RBMEMB);	\
		return this_node;						\
	}									\
										\
	/* Return the node with the next higher key or NULL if no such node	\
	 * exists. */								\
	static inline NODE_T* PREFIX##_next(NODE_T* n)				\
	{									\
		struct rb_node* node = rb_next(&n->NODE_RBMEMB);		\
		NODE_T* this_node;						\
										\
		if(!node)							\
			return NULL;						\
										\
		this_node = rb_entry(node, NODE_T, NODE_RBMEMB);		\
										\
		return this_node;						\
	}									\
										\
	/* Return the node with the next node in postorder traversal or NULL if \
	 * no such node  exists. */						\
	static inline NODE_T* PREFIX##_next_postorder(NODE_T* n)		\
	{									\
		struct rb_node* node = rb_next_postorder(&n->NODE_RBMEMB);	\
		NODE_T* this_node;						\
										\
		if(!node)							\
			return NULL;						\
										\
		this_node = rb_entry(node, NODE_T, NODE_RBMEMB);		\
										\
		return this_node;						\
	}									\
										\
	/* Find the node whose key is needle. Returns NULL if no such node	\
	 * exists. */								\
	static inline NODE_T* PREFIX##_find(TREE_T* t, KEY_T needle)		\
	{									\
		struct rb_node* curr = t->TREE_ROOTMEMB.rb_node;		\
										\
		/* Figure out where to put new node */				\
		while(curr) {							\
			NODE_T* this = container_of(curr, NODE_T, NODE_RBMEMB); \
										\
			if (needle < KEYACC_EXPR(*this))			\
				curr = curr->rb_left;				\
			else if (needle > KEYACC_EXPR(*this))			\
				curr = curr->rb_right;				\
			else							\
				return this;					\
		}								\
										\
		return NULL;							\
	}

#endif
