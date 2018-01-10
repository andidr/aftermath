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

#include "dfg_coordinate_mapping.h"
#include <limits.h>

/* Retrieves the coordinates of the node specified by id. The coordinates are
 * passed in *p. If no mapping exists, the function returns 1, otherwise 0. */
int am_dfg_coordinate_mapping_get_coordinates(
	const struct am_dfg_coordinate_mapping* m,
	long id,
	struct am_point* p)
{
	struct am_dfg_node_coordinate* c;

	if(!(c = am_dfg_coordinate_mapping_bsearch(m, id)))
		return 1;

	p->x = c->pos.x;
	p->y = c->pos.y;

	return 0;

}

/* Sets the x and y coordinate of the node of a node specified by id. Returns 0
 * on success, otherwise 1. */
int am_dfg_coordinate_mapping_set_coordinates(
	struct am_dfg_coordinate_mapping* m,
	long id,
	double x,
	double y)
{
	struct am_dfg_node_coordinate* c;

	/* If there is already a mapping for this id, just update the
	 * coordinates */
	if((c = am_dfg_coordinate_mapping_bsearch(m, id))) {
		c->pos.x = x;
		c->pos.y = y;

		return 0;
	}

	/* Otherwise, add a new entry */
	if(!(c = am_dfg_coordinate_mapping_reserve_sorted(m, id)))
		return 1;

	c->id = id;
	c->pos.x = x;
	c->pos.y = y;

	return 0;
}

void am_dfg_coordinate_mapping_remove_coordinates(
	struct am_dfg_coordinate_mapping* m,
	long id)
{
	am_dfg_coordinate_mapping_remove_sorted(m, id);
}

/* Sets the coordinates of a node from an object notation list node of the
 * form
 *
 *  [id, x, y]
 *
 * Where id is an integer identifying the node whose coordinates are to be set,
 * x is the integer x position and y the integer y position.
 *
 * Returns 0 on success, otherwise 1.
 */
int
am_dfg_coordinate_mapping_from_object_notation_single(
	struct am_dfg_coordinate_mapping* m,
	struct am_object_notation_node_list* lst)
{
	struct am_object_notation_node* iter;
	struct am_object_notation_node_int* iter_int;
	uint64_t ui;
	long id;
	double x = 0;
	double y = 0;
	size_t i = 0;

	if(am_object_notation_node_list_num_items(lst) != 3)
		return 1;

	am_object_notation_for_each_list_item(lst, iter) {
		if(iter->type != AM_OBJECT_NOTATION_NODE_TYPE_INT)
			return 1;

		iter_int = (typeof(iter_int))iter;

		if(i == 0) {
			if((ui = iter_int->value) > LONG_MAX)
				return 1;

			id = (long)ui;
		} else if(i == 1) {
			x = (double)iter_int->value;
		} else if(i == 2) {
			y = (double)iter_int->value;
		}

		i++;
	}

	if(am_dfg_coordinate_mapping_set_coordinates(m, id, x, y))
		return 1;

	return 0;
}

/* Sets the coordinates of a set of nodes from an object notation list node of
 * the form
 *
 *  [[id, x, y], ...]
 *
 * Where id is an integer identifying the node whose coordinates are to be set,
 * x is the integer x position and y the integer y position.
 *
 * Returns 0 on success, otherwise 1.
 */
int
am_dfg_coordinate_mapping_from_object_notation(
	struct am_dfg_coordinate_mapping* m,
	struct am_object_notation_node_list* lst)
{
	struct am_object_notation_node* iter;
	struct am_object_notation_node_list* iter_lst;

	am_object_notation_for_each_list_item(lst, iter) {
		if(iter->type != AM_OBJECT_NOTATION_NODE_TYPE_LIST)
			return 1;

		iter_lst = (typeof(iter_lst))iter;

		if(am_dfg_coordinate_mapping_from_object_notation_single(
			   m, iter_lst))
		{
			return 1;
		}
	}

	return 0;
}
