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

#include "generic.h"

/* Default destructor for pointer samples (e.g., char* aka string) */
void am_dfg_type_generic_free_samples(const struct am_dfg_type* t,
				      size_t num_samples,
				      void* ptr)
{
	void** pptr = ptr;
	void* curr;

	for(size_t i = 0; i < num_samples; i++) {
		curr = pptr[i];
		free(curr);
	}
}
