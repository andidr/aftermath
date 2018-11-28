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

#ifndef AM_DFG_TYPE_PAIR_TIMESTAMP_HIERARCHY_NODE_H
#define AM_DFG_TYPE_PAIR_TIMESTAMP_HIERARCHY_NODE_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/hierarchy.h>

struct am_dfg_type_pair_timestamp_const_hierarchy_node {
	struct am_hierarchy_node* node;
	am_timestamp_t timestamp;
};

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_pair_timestamp_const_hierarchy_node,
	"am::core::pair<am::core::timestamp,const am::core::hierarchy_node*>",
	sizeof(struct am_dfg_type_pair_timestamp_const_hierarchy_node),
	NULL, NULL, NULL, NULL)

AM_DFG_ADD_BUILTIN_TYPES(&am_dfg_type_pair_timestamp_const_hierarchy_node)

#endif
