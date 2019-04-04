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

#include "histogram_attributes.h"
#include <aftermath/core/statistics/histogram.h>

#define AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(T, TPREFIX)			\
	int am_dfg_histogram_##TPREFIX##_attributes_node_process(		\
		struct am_dfg_node* n)						\
	{									\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* pdata = &n->ports[1];			\
		struct am_dfg_port* pleft = &n->ports[2];			\
		struct am_dfg_port* pright = &n->ports[3];			\
		struct am_dfg_port* pnum_bins = &n->ports[4];			\
		struct am_histogram1d_data* hdclone;				\
										\
		size_t nin;							\
										\
		T* oleft = NULL;						\
		T* oright = NULL;						\
		uint64_t* onum_bins = NULL;					\
										\
		struct am_histogram1d_##TPREFIX** histograms;			\
										\
		if(!am_dfg_port_is_connected(pin))				\
			return 0;						\
										\
		histograms = pin->buffer->data;				\
										\
		if((nin = pin->buffer->num_samples) == 0)			\
			return 0;						\
										\
		if(am_dfg_port_is_connected(pdata)) {				\
			for(size_t i = 0; i < nin; i++) {			\
				if(!(hdclone = am_histogram1d_data_clone(&histograms[i]->data))) \
					return 1;				\
										\
				if(am_dfg_buffer_write(pdata->buffer, 1, &hdclone)) {	\
					am_histogram1d_data_destroy(hdclone);	\
					free(hdclone);				\
				}						\
			}							\
		}								\
										\
		if(am_dfg_port_is_connected(pleft)) {				\
			if(!(oleft = am_dfg_buffer_reserve(pleft->buffer, nin)))\
				return 1;					\
										\
			for(size_t i = 0; i < nin; i++)			\
				oleft[i] = histograms[i]->left;		\
		}								\
										\
		if(am_dfg_port_is_connected(pright)) {				\
			if(!(oright = am_dfg_buffer_reserve(			\
				     pright->buffer, nin)))			\
			{							\
				return 1;					\
			}							\
										\
			for(size_t i = 0; i < nin; i++)			\
				oright[i] = histograms[i]->right;		\
		}								\
										\
		if(am_dfg_port_is_connected(pnum_bins)) {			\
			if(!(onum_bins = am_dfg_buffer_reserve(		\
				     pnum_bins->buffer, nin)))			\
			{							\
				return 1;					\
			}							\
										\
			for(size_t i = 0; i < nin; i++) {			\
				if(am_safe_u64_from_size(			\
					   &onum_bins[i],			\
					   histograms[i]->data.num_bins))	\
				{						\
					am_dfg_buffer_shrink(pnum_bins->buffer, \
							     nin);		\
					return 1;				\
				}						\
			}							\
		}								\
										\
		return 0;							\
	}


AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE( int8_t,  int8)
AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(int16_t, int16)
AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(int32_t, int32)
AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(int64_t, int64)

AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE( uint8_t,  uint8)
AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(uint16_t, uint16)
AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(uint32_t, uint32)
AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(uint64_t, uint64)

AM_DFG_IMPL_HISTOGRAM_ATTRIBUTES_NODE(double, double)
