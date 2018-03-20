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

#ifndef AM_DFG_COORDINATE_MAPPING_H
#define AM_DFG_COORDINATE_MAPPING_H

#include <aftermath/render/cairo_extras.h>
#include <aftermath/core/dfg_node.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/object_notation.h>

struct am_dfg_node_coordinate {
	/* The id of the node whose coordinates are provided */
	long id;

	/* The coordinates of the node */
	struct am_point pos;
};

#define AM_DFG_NODE_COORDINATE_ACC_ID(x) (x.id)

AM_DECL_TYPED_ARRAY(am_dfg_coordinate_mapping, struct am_dfg_node_coordinate)

AM_DECL_TYPED_ARRAY_BSEARCH(am_dfg_coordinate_mapping,
			    struct am_dfg_node_coordinate,
			    long,
			    AM_DFG_NODE_COORDINATE_ACC_ID,
			    AM_VALCMP_PTR)

AM_DECL_TYPED_ARRAY_INSERTPOS(am_dfg_coordinate_mapping,
			      struct am_dfg_node_coordinate,
			      long,
			      AM_DFG_NODE_COORDINATE_ACC_ID,
			      AM_VALCMP_PTR)

AM_DECL_TYPED_ARRAY_REMOVE(am_dfg_coordinate_mapping,
			      struct am_dfg_node_coordinate,
			      long)

AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_dfg_coordinate_mapping,
			    struct am_dfg_node_coordinate,
			    long)

int am_dfg_coordinate_mapping_set_coordinates(
	struct am_dfg_coordinate_mapping* m,
	long id,
	double x,
	double y);

int am_dfg_coordinate_mapping_get_coordinates(
	const struct am_dfg_coordinate_mapping* m,
	long id,
	struct am_point* p);

void am_dfg_coordinate_mapping_remove_coordinates(
	struct am_dfg_coordinate_mapping* m,
	long id);

int
am_dfg_coordinate_mapping_from_object_notation(
	struct am_dfg_coordinate_mapping* m,
	struct am_object_notation_node_list* lst);

struct am_object_notation_node*
am_dfg_coordinate_mapping_to_object_notation(
	const struct am_dfg_coordinate_mapping* m);

#endif
