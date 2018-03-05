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

int am_dfg_duration_to_string_node_process(struct am_dfg_node* n)
{
	size_t max_digits = AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 1;
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	struct am_time_offset* durations;
	size_t old_num_samples;
	char* str;

	if(!am_dfg_port_is_connected(pin) || !am_dfg_port_is_connected(pout))
		return 0;

	old_num_samples = pout->buffer->num_samples;
	durations = pin->buffer->data;

	/* Try to convert all samples. If one conversion fails, destroy samples
	 * produced so far in out_err_free. */
	for(size_t i = 0; i < pin->buffer->num_samples; i++) {
		if(!(str = malloc(max_digits+1)))
			goto out_err;

		snprintf(str, AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS,
			 "%s%" AM_TIMESTAMP_T_FMT,
			 (durations[i].sign) ? "-" : "",
			 durations[i].abs);

		if(am_dfg_buffer_write(pout->buffer, 1, &str))
			goto out_err_free;
	}

	return 0;

out_err_free:
	free(str);
out_err:
	am_dfg_buffer_resize(pout->buffer, old_num_samples);
	return 1;
}
