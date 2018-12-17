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
#include <aftermath/core/safe_alloc.h>
#include <string.h>

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

/* Default copy function for samples that do not require a constructor. Copies
 * num_samples samples of type t from ptr_in to ptr_out. The memory region
 * pointed to by ptr_out must have been allocated before the call.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_type_generic_plain_copy_samples(const struct am_dfg_type* t,
					   size_t num_samples,
					   void* ptr_in,
					   void* ptr_out)
{
	size_t size_bytes;

	if(am_size_mul_safe(&size_bytes, t->sample_size, num_samples))
		return 1;

	memcpy(ptr_out, ptr_in, size_bytes);

	return 0;
}
