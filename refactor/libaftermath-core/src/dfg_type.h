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

#ifndef AM_DFG_TYPES_H
#define AM_DFG_TYPES_H

#include <aftermath/core/contrib/linux-kernel/list.h>
#include <stdlib.h>

struct am_dfg_type;

typedef void (*am_dfg_type_destroy_samples_fun_t)(const struct am_dfg_type* t,
						  size_t num_samples,
						  void* ptr);

/* Represents a data type (e.g., for ports) */
struct am_dfg_type {
	/* Name of the type */
	char* name;

	/* Chaining in a type registry */
	struct list_head list;

	/* Size of a sample of this type */
	size_t sample_size;

	/* Function called when an array of sample of this type is to be
	 * destroyed (e.g., a destructor that frees memory not exposed to the
	 * system through sample_size). Can be null if no destruction is needed
	 * prior to freeing the memory. The argument ptr is a pointer to the
	 * first sample and must not be freed. */
	am_dfg_type_destroy_samples_fun_t destroy_samples;
};

int am_dfg_type_init(struct am_dfg_type* t, const char* name, size_t sample_size);
void am_dfg_type_destroy(struct am_dfg_type* t);

#endif
