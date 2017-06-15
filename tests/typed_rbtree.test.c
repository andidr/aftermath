/**
 * Copyright (C) 2017 Andi Drebes <andi.drebes@lip6.fr>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <unit_tests.h>
#include "../src/typed_rbtree.h"
#include "common.h"

struct a_tree {
	struct rb_root root_int;
	struct rb_root root_float;
};

struct a_node {
	struct rb_node node_int;
	struct rb_node node_float;

	int key_int;
	float key_float;
};

#define ACC_INT(x) ((x).key_int)
#define ACC_FLOAT(x) ((x).key_float)

DECL_TYPED_RBTREE_OPS(int_tree,
		      struct a_tree, root_int,
		      struct a_node, node_int, int, ACC_INT)

DECL_TYPED_RBTREE_OPS(float_tree,
		      struct a_tree, root_float,
		      struct a_node, node_float, float, ACC_FLOAT)

UNIT_TEST(empty_test)
{
	struct a_tree t;

	int_tree_init(&t);
	float_tree_init(&t);

	ASSERT_NULL(int_tree_first(&t));
	ASSERT_NULL(int_tree_find(&t, 10));

	ASSERT_NULL(float_tree_first(&t));
	ASSERT_NULL(float_tree_find(&t, 10));
}
END_TEST()

UNIT_TEST(insert_test)
{
	struct a_tree t;
	struct a_node n1;
	struct a_node n1_clone;

	int_tree_init(&t);

	n1.key_int = 1;
	n1_clone.key_int = 1;

	ASSERT_EQUALS(int_tree_insert(&t, &n1), 0);
	ASSERT_EQUALS(int_tree_insert(&t, &n1_clone), 1);

	ASSERT_EQUALS_PTR(int_tree_first(&t), &n1);
	ASSERT_NULL(int_tree_next(&n1));

	ASSERT_EQUALS_PTR(int_tree_find(&t, 1), &n1);
}
END_TEST()

UNIT_TEST(insert_two_roots_test)
{
	struct a_tree t;
	struct a_node n1;

	int_tree_init(&t);
	float_tree_init(&t);

	n1.key_int = 1;
	n1.key_float = 2.0f;

	ASSERT_EQUALS(int_tree_insert(&t, &n1), 0);
	ASSERT_EQUALS_PTR(int_tree_first(&t), &n1);
	ASSERT_NULL(int_tree_next(&n1));
	ASSERT_EQUALS_PTR(int_tree_find(&t, 1), &n1);
	ASSERT_NULL(float_tree_first(&t));
	ASSERT_NULL(float_tree_find(&t, 2.0));

	ASSERT_EQUALS(float_tree_insert(&t, &n1), 0);
	ASSERT_EQUALS_PTR(float_tree_first(&t), &n1);
	ASSERT_EQUALS_PTR(float_tree_find(&t, 2.0), &n1);
}
END_TEST()

UNIT_TEST(insert_many_test)
{
	struct a_tree t;
	size_t num_nodes = 100;
	struct a_node nodes[num_nodes];
	struct a_node* iter;

	int_tree_init(&t);
	float_tree_init(&t);

	for(size_t i = 0; i < num_nodes; i++) {
		nodes[i].key_int = i;
		nodes[i].key_float = num_nodes - i;
		ASSERT_EQUALS(int_tree_insert(&t, &nodes[i]), 0);
		ASSERT_EQUALS(float_tree_insert(&t, &nodes[i]), 0);
	}

	iter = int_tree_first(&t);

	for(size_t i = 0; i < num_nodes; i++) {
		ASSERT_EQUALS_PTR(iter, &nodes[i]);

		if(i < num_nodes-1) {
			iter = int_tree_next(iter);
			ASSERT_NONNULL(iter);
		}
	}

	iter = float_tree_first(&t);

	for(size_t i = 0; i < num_nodes; i++) {
		ASSERT_EQUALS_PTR(iter, &nodes[num_nodes-i-1]);

		if(i < num_nodes-1) {
			iter = float_tree_next(iter);
			ASSERT_NONNULL(iter);
		}
	}
}
END_TEST()

UNIT_TEST(remove_test)
{
	struct a_tree t;
	size_t num_nodes = 100;
	struct a_node nodes[num_nodes];
	struct a_node* iter;

	int_tree_init(&t);

	for(size_t i = 0; i < num_nodes; i++) {
		nodes[i].key_int = i;
		ASSERT_EQUALS(int_tree_insert(&t, &nodes[i]), 0);
	}

	iter = int_tree_first(&t);

	for(size_t i = 0; i < num_nodes; i++) {
		int_tree_remove(&t, &nodes[i]);
		ASSERT_NULL(int_tree_find(&t, i));

		if(i < num_nodes-1) {
			iter = int_tree_first(&t);
			ASSERT_EQUALS_PTR(iter, &nodes[i+1]);
		}
	}
}
END_TEST()

UNIT_TEST_SUITE(typed_rbtree_test)
{
	ADD_TEST(empty_test);
	ADD_TEST(insert_test);
	ADD_TEST(insert_two_roots_test);
	ADD_TEST(insert_many_test);
	ADD_TEST(remove_test);
}
END_TEST_SUITE()
