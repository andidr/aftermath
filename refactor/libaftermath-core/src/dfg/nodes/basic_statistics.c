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

#include <aftermath/core/dfg/nodes/basic_statistics.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/arithmetic.h>

#define AM_DFG_MINMAX_NODE_IMPL(FUN, REPLOP, T, TPREFIX, TPREFIX_CAP)		\
	int am_dfg_##TPREFIX##_##FUN##_node_process(struct am_dfg_node* n)	\
	{									\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* pout = &n->ports[1];			\
		T* in;								\
		T curr;							\
										\
		if(am_dfg_port_activated_and_has_data(pin) &&			\
		   am_dfg_port_activated(pout))				\
		{								\
			in = pin->buffer->data;				\
										\
			curr = in[0];						\
										\
			for(size_t i = 1; i < pin->buffer->num_samples; i++)	\
				if(curr REPLOP in[i])				\
					curr = in[i];				\
										\
			if(am_dfg_buffer_write(pout->buffer, 1, &curr))	\
				return 1;					\
		}								\
										\
		return 0;							\
	}

AM_DFG_MINMAX_NODE_IMPL(min, >,  uint8_t,  uint8,  Uint8)
AM_DFG_MINMAX_NODE_IMPL(min, >, uint16_t, uint16, Uint16)
AM_DFG_MINMAX_NODE_IMPL(min, >, uint32_t, uint32, Uint32)
AM_DFG_MINMAX_NODE_IMPL(min, >, uint64_t, uint64, Uint64)

AM_DFG_MINMAX_NODE_IMPL(min, >, am_timestamp_t, timestamp, Timestamp)
AM_DFG_MINMAX_NODE_IMPL(min, >, double, double, Double)

AM_DFG_MINMAX_NODE_IMPL(min, >,  int8_t,  int8, int8)
AM_DFG_MINMAX_NODE_IMPL(min, >, int16_t, int16, int16)
AM_DFG_MINMAX_NODE_IMPL(min, >, int32_t, int32, int32)
AM_DFG_MINMAX_NODE_IMPL(min, >, int64_t, int64, int64)

AM_DFG_MINMAX_NODE_IMPL(max, <,  uint8_t,  uint8,  Uint8)
AM_DFG_MINMAX_NODE_IMPL(max, <, uint16_t, uint16, Uint16)
AM_DFG_MINMAX_NODE_IMPL(max, <, uint32_t, uint32, Uint32)
AM_DFG_MINMAX_NODE_IMPL(max, <, uint64_t, uint64, Uint64)

AM_DFG_MINMAX_NODE_IMPL(max, <, am_timestamp_t, timestamp, Timestamp)
AM_DFG_MINMAX_NODE_IMPL(max, <, double, double, Double)

AM_DFG_MINMAX_NODE_IMPL(max, <,  int8_t,  int8, int8)
AM_DFG_MINMAX_NODE_IMPL(max, <, int16_t, int16, int16)
AM_DFG_MINMAX_NODE_IMPL(max, <, int32_t, int32, int32)
AM_DFG_MINMAX_NODE_IMPL(max, <, int64_t, int64, int64)

#define AM_DFG_AVG_NODE_IMPL(T, TPREFIX, TPREFIX_CAP, SAFE_SUFFIX)		\
	int am_dfg_##TPREFIX##_avg_node_process(struct am_dfg_node* n)		\
	{									\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* pout = &n->ports[1];			\
		T* in;								\
		T sum;								\
		T avg;								\
										\
		if(am_dfg_port_activated_and_has_data(pin) &&			\
		   am_dfg_port_activated(pout))				\
		{								\
			in = pin->buffer->data;				\
										\
			sum = in[0];						\
										\
			for(size_t i = 1; i < pin->buffer->num_samples; i++) {	\
				if(am_add_safe_##SAFE_SUFFIX(in[i], sum, &sum) != \
				   AM_ARITHMETIC_STATUS_EXACT)			\
				{						\
					return 1;				\
				}						\
			}							\
										\
			avg = sum / ((T)pin->buffer->num_samples);	\
										\
			if(am_dfg_buffer_write(pout->buffer, 1, &avg))		\
				return 1;					\
		}								\
										\
		return 0;							\
	}

AM_DFG_AVG_NODE_IMPL( uint8_t,  uint8,  Uint8,  u8)
AM_DFG_AVG_NODE_IMPL(uint16_t, uint16, Uint16, u16)
AM_DFG_AVG_NODE_IMPL(uint32_t, uint32, Uint32, u32)
AM_DFG_AVG_NODE_IMPL(uint64_t, uint64, Uint64, u64)

AM_DFG_AVG_NODE_IMPL(am_timestamp_t, timestamp, Timestamp, u64)
AM_DFG_AVG_NODE_IMPL(double, double, Double, double)

AM_DFG_AVG_NODE_IMPL( int8_t,  int8, int8,  i8)
AM_DFG_AVG_NODE_IMPL(int16_t, int16, int16, i16)
AM_DFG_AVG_NODE_IMPL(int32_t, int32, int32, i32)
AM_DFG_AVG_NODE_IMPL(int64_t, int64, int64, i64)
