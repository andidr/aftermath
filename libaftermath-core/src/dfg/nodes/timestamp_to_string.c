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

#include "timestamp_to_string.h"
#include <aftermath/core/base_types.h>

#if AM_TIMESTAMP_T_BITS != 64 || AM_TIMESTAMP_T_SIGNED != 0
#error "Assuming am_timestamp_t to be an unsigned 64 bit value, but it isn't"
#endif

int am_dfg_timestamp_to_string_node_init(struct am_dfg_node* n)
{
	struct am_dfg_timestamp_to_string_node* tts = (typeof(tts))n;

	tts->pretty_print = 0;
	tts->max_significant_digits = 3;

	return 0;
}

int am_dfg_timestamp_to_string_node_process(struct am_dfg_node* n)
{
	struct am_dfg_timestamp_to_string_node* tts = (typeof(tts))n;
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	am_timestamp_t* timestamps;
	size_t old_num_samples;
	char* str;
	size_t alloc_bytes;
	size_t gen_len;

	if(!am_dfg_port_is_connected(pin) || !am_dfg_port_is_connected(pout))
		return 0;

	old_num_samples = pout->buffer->num_samples;
	timestamps = pin->buffer->data;

	if(tts->pretty_print) {
		/* If pretty printing: at most the maximum number of digits
		 * before the dot, then the dot, then the maximum number of
		 * significant digits, the si prefix and the terminating '\0'
		 * character. */
		alloc_bytes = AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 1 +
			tts->max_significant_digits + 1 + 1;
	} else {
		alloc_bytes = AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS+1;
	}

	/* Try to convert all samples. If one conversion fails, destroy samples
	 * produced so far in out_err_free. */
	for(size_t i = 0; i < pin->buffer->num_samples; i++) {
		if(!(str = malloc(alloc_bytes)))
			goto out_err;

		if(tts->pretty_print) {
			if(am_siformat_u64(timestamps[i],
					   tts->max_significant_digits,
					   str,
					   alloc_bytes))
			{
				/* Characters were omitted, add final '+' */
				gen_len = strlen(str);

				if(gen_len < alloc_bytes-1) {
					str[gen_len] = '+';
					str[gen_len+1] = '\0';
				} else {
					if(gen_len > 0) {
						str[gen_len-1] = '+';
					} else if(alloc_bytes > 1) {
						str[0] = '+';
						str[1] = '\0';
					}
				}
			}
		} else {
			snprintf(str, AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS,
				 "%" AM_TIMESTAMP_T_FMT, timestamps[i]);
		}

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

int am_dfg_timestamp_to_string_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value)
{
	struct am_dfg_timestamp_to_string_node* tts = (typeof(tts))n;

	if(strcmp(property->name, "pretty_print") == 0) {
		tts->pretty_print = *((int*)value);
		return 0;
	} else if(strcmp(property->name, "max_significant_digits") == 0) {
		tts->max_significant_digits = *((uint8_t*)value);
		return 0;
	}

	return 1;
}

int am_dfg_timestamp_to_string_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value)
{
	struct am_dfg_timestamp_to_string_node* tts = (typeof(tts))n;

	if(strcmp(property->name, "pretty_print") == 0) {
		*value = &tts->pretty_print;
		return 0;
	} else if(strcmp(property->name, "max_significant_digits") == 0) {
		*value = &tts->max_significant_digits;
		return 0;
	}

	return 1;
}

int am_dfg_timestamp_to_string_node_from_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g)
{
	struct am_dfg_timestamp_to_string_node* tts = (typeof(tts))n;
	uint64_t u64val;

	if(am_object_notation_eval_retrieve_uint64(&g->node,
						   "pretty_print",
						   &u64val) == 0)
	{
		tts->pretty_print = u64val ? 1 : 0;
	}

	if(am_object_notation_eval_retrieve_uint64(&g->node,
						   "max_significant_digits",
						   &u64val) == 0)
	{
		tts->max_significant_digits = u64val;
	}

	return 0;
}

int am_dfg_timestamp_to_string_node_to_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g)
{
	struct am_dfg_timestamp_to_string_node* tts = (typeof(tts))n;
	struct am_object_notation_node_member* mformat;
	uint64_t pretty_print = tts->pretty_print;
	uint64_t sig_digits = tts->max_significant_digits;

	mformat = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "pretty_print",
			  AM_OBJECT_NOTATION_BUILD_UINT64, pretty_print,
			AM_OBJECT_NOTATION_BUILD_MEMBER, "max_significant_digits",
			  AM_OBJECT_NOTATION_BUILD_UINT64, sig_digits);

	if(!mformat)
		return 1;

	am_object_notation_node_group_add_member(g, mformat);

	return 0;
}
