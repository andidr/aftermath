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

#include "histogram_builder.h"
#include <aftermath/core/statistics/histogram.h>

/* BITS must be at most 64 */
#define AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(T, TPREFIX, TPREFIXUP, AMPREFIX,	\
					  SIGNPREFIXEMPTY, SIGNPREFIXEMPTYUP,	\
					  SIGNPREFIX, MIN)			\
	int am_dfg_histogram_builder_##TPREFIX##_node_init(			\
		struct am_dfg_node* n)						\
	{									\
		struct am_dfg_histogram_builder_##TPREFIX##_node* hb =		\
			(typeof(hb))n;						\
										\
		hb->num_bins = 100;						\
		hb->num_bins_u64 = 100;					\
		hb->auto_min_max = 1;						\
		hb->min = 0;							\
		hb->max = 0;							\
										\
		return 0;							\
	}									\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_process(			\
		struct am_dfg_node* n)						\
	{									\
		struct am_dfg_histogram_builder_##TPREFIX##_node* hb =		\
			(typeof(hb))n;						\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* pout = &n->ports[1];			\
		struct am_histogram1d_##TPREFIX* h;				\
		T min = TPREFIXUP##_MAX;					\
		T max = MIN;							\
		T* samples_uint;						\
										\
		if(!am_dfg_port_is_connected(pin) ||				\
		   !am_dfg_port_is_connected(pout))				\
		{								\
			return 0;						\
		}								\
										\
		if(!(h = malloc(sizeof(*h))))					\
			goto out_err;						\
										\
		samples_uint = pin->buffer->data;				\
										\
		/* Scan for minimum and maximum value */			\
		if(hb->auto_min_max) {						\
			for(size_t i = 0; i < pin->buffer->num_samples; i++) {	\
				if(samples_uint[i] > max)			\
					max = samples_uint[i];			\
										\
				if(samples_uint[i] < min)			\
					min = samples_uint[i];			\
			}							\
										\
			if(pin->buffer->num_samples == 0) {			\
				min = 0;					\
				max = 0;					\
			}							\
		} else {							\
			min = hb->min;						\
			max = hb->max;						\
		}								\
										\
		if(am_histogram1d_##TPREFIX##_init(				\
			   h, hb->num_bins, min, max,				\
			   AM_HISTOGRAM_BIN_MODE_IGNORE))			\
		{								\
			goto out_err_free;					\
		}								\
										\
		for(size_t i = 0; i < pin->buffer->num_samples; i++)		\
			if(am_histogram1d_##TPREFIX##_add_sample(		\
				   h, samples_uint[i]))			\
			{							\
				goto out_err_destroy;				\
			}							\
										\
		if(am_dfg_buffer_write(pout->buffer, 1, &h))			\
			goto out_err_destroy;					\
										\
		return 0;							\
										\
	out_err_destroy:							\
		am_histogram1d_##TPREFIX##_destroy(h);				\
	out_err_free:								\
		free(h);							\
	out_err:								\
		return 1;							\
	}									\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_set_property(		\
		struct am_dfg_node* n,						\
		const struct am_dfg_property* property,			\
		const void* value)						\
	{									\
		struct am_dfg_histogram_builder_##TPREFIX##_node* hb =		\
			(typeof(hb))n;						\
										\
		if(strcmp(property->name, "num_bins") == 0) {			\
			if(am_safe_size_from_u64(&hb->num_bins,		\
						 *((uint64_t*)value)))		\
				return 1;					\
										\
			hb->num_bins_u64 = *((uint64_t*)value);		\
		} else if(strcmp(property->name, "auto_min_max") == 0) {	\
			hb->auto_min_max = (*((int*)value)) ? 1 : 0;		\
		} else if(strcmp(property->name, "min") == 0) {		\
			hb->min = *((T*)value);				\
		} else if(strcmp(property->name, "max") == 0) {		\
			hb->max = *((T*)value);				\
		}								\
										\
		return 0;							\
	}									\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_get_property(		\
		const struct am_dfg_node* n,					\
		const struct am_dfg_property* property,			\
		void** value)							\
	{									\
		struct am_dfg_histogram_builder_##TPREFIX##_node* hb = 	\
			(typeof(hb))n;						\
										\
		if(strcmp(property->name, "num_bins") == 0)			\
			*value = &hb->num_bins_u64;				\
		else if(strcmp(property->name, "auto_min_max") == 0)		\
			*value = &hb->auto_min_max;				\
		else if(strcmp(property->name, "min") == 0)			\
			*value = &hb->min;					\
		else if(strcmp(property->name, "max") == 0)			\
			*value = &hb->max;					\
		else								\
			return 1;						\
										\
		return 0;							\
	}									\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_from_object_notation(	\
		struct am_dfg_node* n, struct am_object_notation_node_group* g) \
	{									\
		struct am_dfg_histogram_builder_##TPREFIX##_node* hb =		\
			(typeof(hb))n;						\
		uint64_t u64val;						\
		SIGNPREFIXEMPTY##int64_t val;					\
										\
		if(am_object_notation_eval_retrieve_uint64(&g->node,		\
							   "num_bins",		\
							   &u64val) == 0)	\
		{								\
			if(am_safe_size_from_u64(&hb->num_bins, u64val))	\
				return 1;					\
										\
			hb->num_bins_u64 = u64val;				\
		}								\
										\
		if(am_object_notation_eval_retrieve_uint64(&g->node,		\
							   "auto_min_max",	\
							   &u64val) == 0)	\
		{								\
			hb->auto_min_max = (u64val) ? 1 : 0;			\
		}								\
										\
		if(am_object_notation_eval_retrieve_##SIGNPREFIXEMPTY##int64(&g->node,\
							   "min",		\
							   &val) == 0)		\
		{								\
			if(am_safe_##AMPREFIX##_from_##SIGNPREFIX##64(&hb->min, \
								      val))	\
			{							\
				return 1;					\
			}							\
		}								\
										\
		if(am_object_notation_eval_retrieve_##SIGNPREFIXEMPTY##int64(&g->node,\
							   "max",		\
							   &val) == 0)		\
		{								\
			if(am_safe_##AMPREFIX##_from_##SIGNPREFIX##64(&hb->max, \
								      val))	\
			{							\
				return 1;					\
			}							\
		}								\
										\
										\
		return 0;							\
	}									\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_to_object_notation(	\
		struct am_dfg_node* n, struct am_object_notation_node_group* g) \
	{									\
		struct am_dfg_histogram_builder_##TPREFIX##_node* hb =	\
			(typeof(hb))n;						\
										\
		return am_object_notation_node_group_build_add_members(	\
			g,							\
			AM_OBJECT_NOTATION_BUILD_MEMBER, "num_bins",		\
			  AM_OBJECT_NOTATION_BUILD_UINT64, hb->num_bins_u64,	\
			AM_OBJECT_NOTATION_BUILD_MEMBER, "auto_min_max",	\
			  AM_OBJECT_NOTATION_BUILD_UINT64,			\
				(uint64_t)hb->auto_min_max,			\
			AM_OBJECT_NOTATION_BUILD_MEMBER, "min",		\
			  AM_OBJECT_NOTATION_BUILD_##SIGNPREFIXEMPTYUP##INT64,	\
			       (SIGNPREFIXEMPTY##int64_t)hb->min,		\
			AM_OBJECT_NOTATION_BUILD_MEMBER, "max",		\
			  AM_OBJECT_NOTATION_BUILD_##SIGNPREFIXEMPTYUP##INT64,	\
			       (SIGNPREFIXEMPTY##int64_t)hb->max);		\
	}

AM_DFG_HISTOGRAM_BUILDER_IMPL_INT( int8_t,  int8,  INT8,  i8, , , i, INT8_MIN)
AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(int16_t, int16, INT16, i16, , , i, INT16_MIN)
AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(int32_t, int32, INT32, i32, , , i, INT32_MIN)
AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(int64_t, int64, INT64, i64, , , i, INT64_MIN)

AM_DFG_HISTOGRAM_BUILDER_IMPL_INT( uint8_t,  uint8,  UINT8,  u8, u, U, u, 0)
AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(uint16_t, uint16, UINT16, u16, u, U, u, 0)
AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(uint32_t, uint32, UINT32, u32, u, U, u, 0)
AM_DFG_HISTOGRAM_BUILDER_IMPL_INT(uint64_t, uint64, UINT64, u64, u, U, u, 0)
