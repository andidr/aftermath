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

#include "interval.h"
#include <string.h>

/* Creates an array of intervals *out with *m elements from an array in of n
 * intervals, in which all overlapping intervals of in have been merged.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_intervals_merge_overlapping(const struct am_interval* in, size_t n,
				   struct am_interval** out, size_t* m)
{
	struct am_interval* sorted;
	size_t i = 0;
	size_t j = 1;
	void* tmp;

	if(!(sorted = calloc(sizeof(struct am_interval), n)))
		return 1;

	/* Multiplication is overflow-safe here, since calloc succeeded */
	memcpy(sorted, in, n * sizeof(struct am_interval));

	am_qsort_intervals_start(sorted, n);

	while(j < n) {
		/* Overlap:
		 *
		 *  sorted[i]: [      ]
		 *  sorted[j]:     [    ]
		 */
		if(sorted[j].start <= sorted[i].end) {
			sorted[i].end = sorted[j].end;
		} else {
			if(j != i+1)
				sorted[i+1] = sorted[j];

			i++;
		}

		j++;
	}

	*m = i+1;
	*out = sorted;

	if(*m != n) {
		/* Try to shrink */
		if((tmp = realloc(sorted, (*m) * sizeof(struct am_interval))))
			*out = tmp;
	}

	return 0;
}
