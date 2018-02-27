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

#include <aftermath/core/dfg_node.h>

/* Undef the default no-ops */
#undef AM_DFG_DECL_BUILTIN_NODE_TYPE_SWITCH
#undef AM_DFG_ADD_BUILTIN_NODE_TYPES_SWITCH

/* Generates the static port definitions and the node definitions from an
 * invocation of AM_DFG_DECL_BUILTIN_NODE_TYPE. See
 * AM_DFG_DECL_BUILTIN_NODE_TYPE for documentation. */
#define AM_DFG_DECL_BUILTIN_NODE_TYPE_SWITCH(ID, NODE_TYPE_NAME,	\
					     NODE_TYPE_HRNAME,		\
					     INSTANCE_SIZE,		\
					     FUNCTIONS, ...)		\
	static struct am_dfg_static_port_type_def ID##_ports[] = {	\
		__VA_ARGS__						\
	};								\
									\
	static struct am_dfg_static_node_type_def ID = {		\
		.name = NODE_TYPE_NAME,				\
		.hrname = NODE_TYPE_HRNAME,				\
		.instance_size = INSTANCE_SIZE,			\
		.functions = FUNCTIONS,				\
		.num_ports = AM_ARRAY_SIZE(ID##_ports),		\
		.ports = ID##_ports					\
	};

/* Generates a NULL-terminated list of static node type definitions for a header
 * file from its invocation of AM_DFG_ADD_BUILTIN_NODE_TYPES. The macro
 * DEFS_NAME() must be defined prior to the invocation, e.g., before inclusion
 * of the header. */
#define AM_DFG_ADD_BUILTIN_NODE_TYPES_SWITCH(...)			\
	static struct am_dfg_static_node_type_def* DEFS_NAME()[] = {	\
		__VA_ARGS__, NULL					\
	};

/* Use the definitions above as replacements for the default no-ops of
 * AM_DFG_DECL_BUILTIN_NODE_TYPE and AM_DFG_ADD_BUILTIN_NODE_TYPES. */
#define AM_DFG_GEN_BUILTIN_NODES
