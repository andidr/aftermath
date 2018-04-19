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

#include "string_concat.h"
#include <aftermath/core/base_types.h>
#include <aftermath/core/safe_alloc.h>

static int
am_dfg_string_concat_node_set_separator_string(struct am_dfg_string_concat_node* f,
					    const char* concat);

int am_dfg_string_concat_node_init(struct am_dfg_node* n)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;

	f->separator = NULL;

	return am_dfg_string_concat_node_set_separator_string(f, "");
}

void am_dfg_string_concat_node_destroy(struct am_dfg_node* n)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;

	free(f->separator);
}

static int
am_dfg_string_concat_node_set_separator_string(struct am_dfg_string_concat_node* f,
					    const char* separator)
{
	size_t len = strlen(separator);
	size_t alloc_len;
	char* tmp;

	if(am_size_add_safe(&alloc_len, len, 1))
		return 1;

	if(!(tmp = realloc(f->separator, alloc_len)))
		return 1;

	f->separator = tmp;
	f->separator_len = len;

	memcpy(f->separator, separator, alloc_len);

	return 0;
}

int am_dfg_string_concat_node_process(struct am_dfg_node* n)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	size_t nsamples;
	size_t alloc_size = 0;
	size_t nsep = 0;
	size_t nsep_chars;
	size_t pos = 0;
	size_t len;
	char** in;
	char* str;

	if(!am_dfg_port_activated(pin) || !am_dfg_port_activated(pout))
		return 0;

	nsamples = pin->buffer->num_samples;

	in = pin->buffer->data;

	/* Count total number of characters */
	for(size_t i = 0; i < nsamples; i++) {
		if(am_size_inc_safe(&alloc_size, strlen(in[i])))
			return 1;
	}

	if(nsamples != 0)
		nsep = nsamples - 1;

	if(am_size_mul_safe(&nsep_chars, nsep, f->separator_len))
		return 1;

	if(am_size_inc_safe(&alloc_size, nsep_chars))
		return 1;

	/* Terminating zero */
	if(am_size_inc_safe(&alloc_size, 1))
		return 1;

	if(!(str = malloc(alloc_size)))
		return 1;

	for(size_t i = 0; i < nsamples; i++) {
		/* All size_t arithmetic is safe here; already made sure that
		 * size_t doesn't overflow */
		len = strlen(in[i]);

		memcpy(&str[pos], in[i], len);
		pos += len;

		if(i != nsamples - 1) {
			memcpy(&str[pos], f->separator, f->separator_len);
			pos += f->separator_len;
		}
	}

	str[pos] = '\0';

	if(am_dfg_buffer_write(pout->buffer, 1, &str)) {
		free(str);
		return 1;
	}

	return 0;
}

int am_dfg_string_concat_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;
	char* const*  pstr = value;

	if(strcmp(property->name, "separator") != 0)
		return 1;

	return am_dfg_string_concat_node_set_separator_string(f, *pstr);
}

int am_dfg_string_concat_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;

	*value = f->separator;

	return 0;
}

int am_dfg_string_concat_node_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;
	const char* separator;

	if(am_object_notation_eval_retrieve_string(
		   &g->node, "separator", &separator) == 0)
	{
		if(am_dfg_string_concat_node_set_separator_string(f, separator))
			return 1;
	}

	return 0;
}

int am_dfg_string_concat_node_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_string_concat_node* f = (typeof(f))n;
	struct am_object_notation_node_member* mseparator;

	mseparator = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "separator",
			AM_OBJECT_NOTATION_BUILD_STRING, f->separator);

	if(!mseparator)
		return 1;

	am_object_notation_node_group_add_member(g, mseparator);

	return 0;
}
