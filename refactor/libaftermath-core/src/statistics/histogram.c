/**
 * Author: Andi Drebes <andi.drebes@lip6.fr>
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

#include "histogram.h"
#include "../safe_alloc.h"

int am_histogram1d_data_init(struct am_histogram1d_data* hd, size_t num_bins)
{
	void* tmp;

	if(!(tmp = calloc(num_bins, sizeof(*hd->bins))))
		return 1;

	hd->bins = tmp;
	hd->num_bins = num_bins;

	return 0;
}

struct am_histogram1d_data*
am_histogram1d_data_clone(struct am_histogram1d_data* hd)
{
	struct am_histogram1d_data* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	if(am_histogram1d_data_init(ret, hd->num_bins)) {
		free(ret);
		return NULL;
	}

	memcpy(ret->bins, hd->bins, sizeof(hd->bins[0]) * hd->num_bins);

	return ret;
}

void am_histogram1d_data_destroy(struct am_histogram1d_data* hd)
{
	free(hd->bins);
}
