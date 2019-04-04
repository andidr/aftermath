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

#include "histogram_data.h"
#include <aftermath/core/statistics/histogram.h>

void am_dfg_type_histogram1d_data_free_samples(const struct am_dfg_type* t,
					       size_t num_samples,
					       void* ptr)
{
	struct am_histogram1d_data** phd = ptr;

	for(size_t i = 0; i < num_samples; i++) {
		am_histogram1d_data_destroy(phd[i]);
		free(phd[i]);
	}
}

int am_dfg_type_histogram1d_data_copy_samples(const struct am_dfg_type* t,
					      size_t num_samples,
					      void* ptr_in,
					      void* ptr_out)
{
	struct am_histogram1d_data** hd_in = ptr_in;
	struct am_histogram1d_data** hd_out = ptr_out;

	for(size_t i = 0; i < num_samples; i++) {
		if(!(hd_out[i] = am_histogram1d_data_clone(hd_in[i]))) {
			for(size_t j = 0; j < i; j++) {
				am_histogram1d_data_destroy(hd_out[j]);
				free(hd_out[j]);
			}

			return 1;
		}
	}

	return 0;
}
