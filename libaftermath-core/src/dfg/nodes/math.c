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

#include <aftermath/core/dfg/nodes/math.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/arithmetic.h>

#define AM_DFG_ADDSUB_NODE_IMPL(FUN, T, TPREFIX, TPREFIX_CAP, SAFE_SUFFIX)	\
	int am_dfg_##TPREFIX##_##FUN##_node_process(struct am_dfg_node* n)	\
	{									\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* pout = &n->ports[1];			\
		T* in;								\
		T accu = 0;							\
										\
		if(am_dfg_port_activated_and_has_data(pin) &&			\
		   am_dfg_port_activated(pout))				\
		{								\
			in = pin->buffer->data;				\
										\
			for(size_t i = 0; i < pin->buffer->num_samples; i++) {	\
				if(am_##FUN##_safe_##SAFE_SUFFIX(in[i], accu, &accu) != \
				   AM_ARITHMETIC_STATUS_EXACT)			\
				{						\
					return 1;				\
				}						\
			}							\
										\
			if(am_dfg_buffer_write(pout->buffer, 1, &accu))	\
				return 1;					\
		}								\
										\
		return 0;							\
	}

AM_DFG_ADDSUB_NODE_IMPL(add,  uint8_t,  uint8,  Uint8,  u8)
AM_DFG_ADDSUB_NODE_IMPL(add, uint16_t, uint16, Uint16, u16)
AM_DFG_ADDSUB_NODE_IMPL(add, uint32_t, uint32, Uint32, u32)
AM_DFG_ADDSUB_NODE_IMPL(add, uint64_t, uint64, Uint64, u64)

AM_DFG_ADDSUB_NODE_IMPL(add, am_timestamp_t, timestamp, Timestamp, u64)
AM_DFG_ADDSUB_NODE_IMPL(add, double, double, Double, double)

AM_DFG_ADDSUB_NODE_IMPL(add,  int8_t,  int8, int8,  i8)
AM_DFG_ADDSUB_NODE_IMPL(add, int16_t, int16, int16, i16)
AM_DFG_ADDSUB_NODE_IMPL(add, int32_t, int32, int32, i32)
AM_DFG_ADDSUB_NODE_IMPL(add, int64_t, int64, int64, i64)

AM_DFG_ADDSUB_NODE_IMPL(sub,  uint8_t,  uint8,  Uint8,  u8)
AM_DFG_ADDSUB_NODE_IMPL(sub, uint16_t, uint16, Uint16, u16)
AM_DFG_ADDSUB_NODE_IMPL(sub, uint32_t, uint32, Uint32, u32)
AM_DFG_ADDSUB_NODE_IMPL(sub, uint64_t, uint64, Uint64, u64)

AM_DFG_ADDSUB_NODE_IMPL(sub, am_timestamp_t, timestamp, Timestamp, u64)
AM_DFG_ADDSUB_NODE_IMPL(sub, double, double, Double, double)

AM_DFG_ADDSUB_NODE_IMPL(sub,  int8_t,  int8, int8,  i8)
AM_DFG_ADDSUB_NODE_IMPL(sub, int16_t, int16, int16, i16)
AM_DFG_ADDSUB_NODE_IMPL(sub, int32_t, int32, int32, i32)
AM_DFG_ADDSUB_NODE_IMPL(sub, int64_t, int64, int64, i64)
