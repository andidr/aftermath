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

#include <aftermath/core/dfg/nodes/int_constant.h>

#define AM_DFG_INT_CONSTANT_NODE_IMPL(TPREFIX, TPREFIX_UP, T, SIGNCHAR,	\
				      SIGNCHAR_UP, TMIN, TMAX)			\
	int am_dfg_##TPREFIX##_constant_node_init(struct am_dfg_node* n)	\
	{									\
		struct am_dfg_##TPREFIX##_constant_node* in =			\
			(struct am_dfg_##TPREFIX##_constant_node*)n;		\
										\
		in->value = 0;							\
		in->num_samples = 1;						\
		in->num_samples64 = 1;						\
										\
		return 0;							\
	}									\
										\
	int am_dfg_##TPREFIX##_constant_node_process(struct am_dfg_node* n)	\
	{									\
		struct am_dfg_##TPREFIX##_constant_node* in =			\
			(struct am_dfg_##TPREFIX##_constant_node*)n;		\
		T* out;							\
										\
		if(am_dfg_port_activated(&n->ports[0])) {			\
			if(!(out = am_dfg_buffer_reserve(			\
				     n->ports[0].buffer, in->num_samples)))	\
			{							\
				return 1;					\
			}							\
										\
			for(size_t i = 0; i < in->num_samples; i++)		\
				out[i] = in->value;				\
		}								\
										\
		return 0;							\
	}									\
										\
	int am_dfg_##TPREFIX##_constant_node_set_property(			\
		struct am_dfg_node* n,						\
		const struct am_dfg_property* property,			\
		const void* value)						\
	{									\
		struct am_dfg_##TPREFIX##_constant_node* in =			\
			(struct am_dfg_##TPREFIX##_constant_node*)n;		\
										\
		if(strcmp(property->name, "value") == 0) {			\
			in->value = *((T*)value);				\
			return 0;						\
		} else if(strcmp(property->name, "num_samples") == 0) {	\
			if(am_safe_size_from_u64(				\
				   &in->num_samples, *((uint64_t*)value)))	\
			{							\
				return 1;					\
			}							\
										\
			in->num_samples64 = *((uint64_t*)value);		\
										\
			return 0;						\
		}								\
										\
		return 1;							\
	}									\
										\
	int am_dfg_##TPREFIX##_constant_node_get_property(			\
		const struct am_dfg_node* n,					\
		const struct am_dfg_property* property,			\
		void** value)							\
	{									\
		struct am_dfg_##TPREFIX##_constant_node* in =			\
			(struct am_dfg_##TPREFIX##_constant_node*)n;		\
										\
		if(strcmp(property->name, "value") == 0) {			\
			*value = &in->value;					\
			return 0;						\
		} else if(strcmp(property->name, "num_samples") == 0) {	\
			*value = &in->num_samples64;				\
			return 0;						\
		}								\
										\
		return 1;							\
	}									\
	int am_dfg_##TPREFIX##_constant_node_from_object_notation(		\
		struct am_dfg_node* n,						\
		struct am_object_notation_node_group* g)			\
	{									\
		struct am_dfg_##TPREFIX##_constant_node* in =			\
			(struct am_dfg_##TPREFIX##_constant_node*)n;		\
		SIGNCHAR##int64_t val64;					\
		uint64_t num_samples64;					\
										\
		if(am_object_notation_eval_retrieve_##SIGNCHAR##int64(		\
			   &g->node, "value", &val64) == 0)			\
		{								\
			if(val64 > TMAX || val64 < TMIN)			\
				return 1;					\
										\
			in->value = val64;					\
		}								\
										\
		if(am_object_notation_eval_retrieve_uint64(			\
			   &g->node, "num_samples", &num_samples64) == 0)	\
		{								\
			if(am_safe_size_from_u64(				\
				   &in->num_samples, num_samples64))		\
			{							\
				return 1;					\
			}							\
										\
			in->num_samples64 = num_samples64;			\
		}								\
										\
		return 0;							\
	}									\
										\
	int am_dfg_##TPREFIX##_constant_node_to_object_notation(		\
		struct am_dfg_node* n,						\
		struct am_object_notation_node_group* g)			\
	{									\
		struct am_dfg_##TPREFIX##_constant_node* in =			\
			(struct am_dfg_##TPREFIX##_constant_node*)n;		\
		SIGNCHAR##int64_t val64 = in->value;				\
										\
		return am_object_notation_node_group_build_add_members(	\
			g,							\
			AM_OBJECT_NOTATION_BUILD_MEMBER, "value",		\
			AM_OBJECT_NOTATION_BUILD_##SIGNCHAR_UP##INT64, val64,	\
			AM_OBJECT_NOTATION_BUILD_MEMBER, "num_samples",	\
			AM_OBJECT_NOTATION_BUILD_UINT64, in->num_samples64);	\
	}

AM_DFG_INT_CONSTANT_NODE_IMPL(uint8,  UINT8,  uint8_t,  u, U, 0,  UINT8_MAX)
AM_DFG_INT_CONSTANT_NODE_IMPL(uint16, UINT16, uint16_t, u, U, 0, UINT16_MAX)
AM_DFG_INT_CONSTANT_NODE_IMPL(uint32, UINT32, uint32_t, u, U, 0, UINT32_MAX)
AM_DFG_INT_CONSTANT_NODE_IMPL(uint64, UINT64, uint64_t, u, U, 0, UINT64_MAX)

AM_DFG_INT_CONSTANT_NODE_IMPL(int8,  INT8,  int8_t  ,,,  INT8_MIN,  INT8_MAX)
AM_DFG_INT_CONSTANT_NODE_IMPL(int16, INT16, int16_t ,,, INT16_MIN, INT16_MAX)
AM_DFG_INT_CONSTANT_NODE_IMPL(int32, INT32, int32_t ,,, INT32_MIN, INT32_MAX)
AM_DFG_INT_CONSTANT_NODE_IMPL(int64, INT64, int64_t ,,, INT64_MIN, INT64_MAX)
