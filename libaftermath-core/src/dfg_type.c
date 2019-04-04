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

#include <aftermath/core/dfg_type.h>
#include <string.h>

/* Initialize a DFG type. The sample size is indicated in bytes. Returns 0 on
 * success, otherwise 1. */
int am_dfg_type_init(struct am_dfg_type* t, const char* name, size_t sample_size)
{
	if(!(t->name = strdup(name)))
		return 1;

	t->sample_size = sample_size;
	t->destroy_samples = NULL;
	t->copy_samples = NULL;

	INIT_LIST_HEAD(&t->list);

	return 0;
}

/* Destroy a type */
void am_dfg_type_destroy(struct am_dfg_type* t)
{
	free(t->name);
}
