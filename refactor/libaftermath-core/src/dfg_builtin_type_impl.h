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
#undef AM_DFG_DECL_BUILTIN_TYPE_SWITCH
#undef AM_DFG_ADD_BUILTIN_TYPES_SWITCH

/* Generates the static type definition from an invocation of
 * AM_DFG_DECL_BUILTIN_TYPE. See AM_DFG_DECL_BUILTIN_TYPE for documentation. */
#define AM_DFG_DECL_BUILTIN_TYPE_SWITCH(ID, NAME, INSTANCE_SIZE,	\
					ARRAY_DESTRUCTOR_FUN,		\
					TO_STRING_FUN,			\
					FROM_STRING_FUN,		\
					CHECK_STRING_FUN)		\
	static struct am_dfg_static_type_def ID = {			\
		.name = NAME,						\
		.sample_size = INSTANCE_SIZE,				\
		.destroy_samples = ARRAY_DESTRUCTOR_FUN,		\
		.to_string = TO_STRING_FUN,				\
		.from_string = FROM_STRING_FUN,			\
		.check_string = CHECK_STRING_FUN			\
	};

/* Generates a NULL-terminated list of static type definitions for a header file
 * from its invocation of AM_DFG_ADD_BUILTIN_TYPES. The macro DEFS_NAME() must
 * be defined prior to the invocation, e.g., before inclusion of the header. */
#define AM_DFG_ADD_BUILTIN_TYPES_SWITCH(...)				\
	static struct am_dfg_static_type_def* DEFS_NAME()[] = {	\
		__VA_ARGS__, NULL					\
	};

/* Use the definitions above as replacements for the default no-ops of
 * AM_DFG_DECL_BUILTIN_TYPE and AM_DFG_ADD_BUILTIN_TYPES. */
#define AM_DFG_GEN_BUILTIN_TYPES
