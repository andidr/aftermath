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

#include <aftermath/core/hierarchy.h>
#include <stdio.h>

/* Initialize a hierarchy without duplicating the string pointed to by "name" by
 * transferring ownership of the allocated space to the hierarchy instance. */
void
am_hierarchy_init_nodup(struct am_hierarchy* h, char* name, am_hierarchy_id_t id)
{
	h->root = NULL;
	h->id = id;
	h->name = name;
}

/* Initialize a hierarchy. Returns 0 on success, 1 otherwise. */
int
am_hierarchy_init(struct am_hierarchy* h, const char* name, am_hierarchy_id_t id)
{
	char* namedup;

	if(!(namedup = strdup(name)))
		return 1;

	am_hierarchy_init_nodup(h, namedup, id);

	return 0;
}

void am_hierarchy_destroy(struct am_hierarchy* h)
{
	if(h->root) {
		am_hierarchy_node_destroy(h->root);
		free(h->root);
	}

	free(h->name);
}

/* Dump a hierarchy node to stdout. The indent indicates how many spaced should
 * be printed before the characters describing the node. */
void am_hierarchy_node_dump(struct am_hierarchy_node* h, size_t indent)
{
	struct am_hierarchy_node* c;

	for(size_t i = 0; i < indent; i++)
		printf("%c", ' ');

	printf("%s\n", h->name);

	am_hierarchy_node_for_each_child(h, c)
		am_hierarchy_node_dump(c, indent+1);
}

/* Dump a hierarchy to stdout. */
void am_hierarchy_dump(struct am_hierarchy* h)
{
	printf("%s\n", h->name);

	if(h->root)
		am_hierarchy_node_dump(h->root, 1);
}

/* Initialize a hierarchy node. Name can be NULL. Returns 0 on success,
 * otherwise 1. */
void am_hierarchy_node_init_nodup(struct am_hierarchy_node* hn,
				  am_hierarchy_node_id_t id,
				  char* name)
{
	hn->name = NULL;
	hn->parent = NULL;
	hn->id = id;
	hn->num_descendants = 0;

	INIT_LIST_HEAD(&hn->siblings);
	INIT_LIST_HEAD(&hn->children);

	if(name)
		am_hierarchy_node_set_name_nodup(hn, name);

	am_event_mapping_init(&hn->event_mapping);
}

/* Initialize a hierarchy node. Name can be NULL. Returns 0 on success,
 * otherwise 1. */
int am_hierarchy_node_init(struct am_hierarchy_node* hn,
			   am_hierarchy_node_id_t id,
			   const char* name)
{
	am_hierarchy_node_init_nodup(hn, id, NULL);

	if(name) {
		if(am_hierarchy_node_set_name(hn, name))
			return 1;
	}

	return 0;
}

/* Destroy a node and all of its children */
void am_hierarchy_node_destroy(struct am_hierarchy_node* hn)
{
	struct am_hierarchy_node* iter;
	struct am_hierarchy_node* tmp;

	am_hierarchy_node_for_each_child_safe(hn, iter, tmp) {
		am_hierarchy_node_destroy(iter);
		free(iter);
	}

	am_event_mapping_destroy(&hn->event_mapping);

	free(hn->name);
}

/*
 * Set the name of a hierarchy node by transferring ownership for the memory
 * reserved for the name to the node (i.e., the string is not copied). Returns 0
 * on success, otherwise 1.
 */
void am_hierarchy_node_set_name_nodup(struct am_hierarchy_node* hn, char* name)
{
	free(hn->name);
	hn->name = name;
}

/*
 * Set the name of a hierarchy node. Returns 0 on success, otherwise 1.
 */
int am_hierarchy_node_set_name(struct am_hierarchy_node* hn, const char* name)
{
	char* tmp;

	if(!(tmp = strdup(name)))
		return 1;

	am_hierarchy_node_set_name_nodup(hn, tmp);

	return 0;
}

/* Returns the number of arcs from the root to n. */
unsigned int am_hierarchy_node_depth(struct am_hierarchy_node* n)
{
	unsigned int depth = 0;

	while(n->parent) {
		depth++;
		n = n->parent;
	}

	return depth;
}

/* Iterates over each descendant of the node n and invokes the function cb until
 * no nodes are left or cb has indicated to stop iterating.
 *
 * Returns 1 if the callback function has indicated that the traversal should be
 * stopped, otherwise 0.
 */
int am_hierarchy_node_for_each_descendant(const struct am_hierarchy* h,
					  struct am_hierarchy_node* n,
					  am_hierarchy_node_callback_fun_t cb,
					  void* data)
{
	struct am_hierarchy_node* c;

	am_hierarchy_node_for_each_child(n, c) {
		if(cb(h, c, data) == AM_HIERARCHY_NODE_CALLBACK_STATUS_STOP)
			return 1;

		if(am_hierarchy_node_for_each_descendant(h, c, cb, data))
			return 1;
	}

	return 0;
}

/* Same as am_hierarchy_node_for_each_descendant, but also invokes cb for n
 * itself.
 */
int am_hierarchy_node_for_each_descendant_and_self(
	const struct am_hierarchy* h,
	struct am_hierarchy_node* n,
	am_hierarchy_node_callback_fun_t cb,
	void* data)
{
	if(cb(h, n, data) == AM_HIERARCHY_NODE_CALLBACK_STATUS_STOP)
		return 1;

	return am_hierarchy_node_for_each_descendant(h, n, cb, data);
}

/* Iterates over each node of the hierarchy h and invokes the function cb until
 * no nodes are left or cb has indicated to stop iterating.
 *
 * Returns 1 if the callback function has indicated that the traversal should be
 * stopped, otherwise 0.
 */
int am_hierarchy_for_each_node(const struct am_hierarchy* h,
			       am_hierarchy_node_callback_fun_t cb,
			       void* data)
{
	if(!h->root)
		return 0;

	return am_hierarchy_node_for_each_descendant_and_self(h, h->root, cb, data);
}
