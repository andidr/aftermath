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
#include "../src/hierarchy.h"

UNIT_TEST(create_node_test)
{
	struct hierarchy_node hn;

	ASSERT_EQUALS(hierarchy_node_init(&hn, 0, "test"), 0);
	ASSERT_FALSE(hierarchy_node_has_children(&hn));
	ASSERT_EQUALS(hn.num_descendants, 0);
	hierarchy_node_destroy(&hn);
}
END_TEST()

static struct hierarchy_node* build_tree(size_t degree, size_t depth)
{
	struct hierarchy_node* root;
	struct hierarchy_node* child;

	root = malloc(sizeof(struct hierarchy_node));
	ASSERT_NONNULL(root);

	ASSERT_EQUALS(hierarchy_node_init(root, 0, "node"), 0);

	if(depth > 0) {
		for(size_t i = 0; i < degree; i++) {
			child = build_tree(degree, depth-1);
			hierarchy_node_add_child(root, child);
			ASSERT_EQUALS(child->parent, root);
		}
	}

	return root;
}

UNIT_TEST(tree_test)
{
	struct hierarchy_node* root;

	root = build_tree(4, 3);
	ASSERT_EQUALS(root->num_descendants, 85-1);
	hierarchy_node_destroy(root);
	free(root);

	root = build_tree(7, 4);
	ASSERT_EQUALS(root->num_descendants, 2801-1);
	hierarchy_node_destroy(root);
	free(root);
}
END_TEST()

UNIT_TEST_SUITE(hierarchy_test)
{
	ADD_TEST(create_node_test);
	ADD_TEST(tree_test);
}
END_TEST_SUITE()
