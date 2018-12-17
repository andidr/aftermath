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

#include "duration_to_string.h"
#include <aftermath/core/base_types.h>
#include <aftermath/core/in_memory.h>

#define AM_DECL_INT_TO_STRING_NODE_IMPL(T, SUFFIX, PRSYM, MAX_LEN)		\
	int am_dfg_##SUFFIX##_to_string_node_process(struct am_dfg_node* n)	\
	{									\
		size_t max_len = MAX_LEN;					\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* pout = &n->ports[1];			\
		T* in;								\
		size_t old_num_samples;					\
		char* str;							\
										\
		if(!am_dfg_port_activated(pin) || !am_dfg_port_activated(pout)) \
			return 0;						\
										\
		old_num_samples = pout->buffer->num_samples;			\
		in = pin->buffer->data;					\
										\
		/* Try to convert all samples. If one conversion fails, destroy \
		 * samples produced so far in out_err_free. */			\
		for(size_t i = 0; i < pin->buffer->num_samples; i++) {		\
			if(!(str = malloc(max_len+1)))				\
				goto out_err;					\
										\
			snprintf(str, max_len+1, "%" PRSYM, in[i]);		\
										\
			if(am_dfg_buffer_write(pout->buffer, 1, &str))		\
				goto out_err_free;				\
		}								\
										\
		return 0;							\
										\
	out_err_free:								\
		free(str);							\
	out_err:								\
		am_dfg_buffer_resize(pout->buffer, old_num_samples);		\
		return 1;							\
	}

AM_DECL_INT_TO_STRING_NODE_IMPL(uint8_t,  uint8,  PRIu8,  3)
AM_DECL_INT_TO_STRING_NODE_IMPL(uint16_t, uint16, PRIu16, 5)
AM_DECL_INT_TO_STRING_NODE_IMPL(uint32_t, uint32, PRIu32, 10)
AM_DECL_INT_TO_STRING_NODE_IMPL(uint64_t, uint64, PRIu64, 20)

AM_DECL_INT_TO_STRING_NODE_IMPL(int8_t,  int8,  PRId8,  4)
AM_DECL_INT_TO_STRING_NODE_IMPL(int16_t, int16, PRId16, 6)
AM_DECL_INT_TO_STRING_NODE_IMPL(int32_t, int32, PRId32, 11)
AM_DECL_INT_TO_STRING_NODE_IMPL(int64_t, int64, PRId64, 21)
