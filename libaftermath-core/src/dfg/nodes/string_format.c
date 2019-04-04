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

#include "string_format.h"
#include <aftermath/core/base_types.h>
#include <aftermath/core/safe_alloc.h>

static int
am_dfg_string_format_node_set_format_string(struct am_dfg_string_format_node* f,
					    const char* format);

int am_dfg_string_format_node_init(struct am_dfg_node* n)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;

	f->format = NULL;

	return am_dfg_string_format_node_set_format_string(f, "%s");
}

void am_dfg_string_format_node_destroy(struct am_dfg_node* n)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;

	free(f->format);
}

static int
am_dfg_string_format_node_set_format_string(struct am_dfg_string_format_node* f,
					    const char* format)
{
	int last_percent = 0;
	size_t len = strlen(format);
	size_t alloc_len;
	char* tmp;

	if(am_size_add_safe(&alloc_len, len, 1))
		return 1;

	if(!(tmp = realloc(f->format, alloc_len)))
		return 1;

	f->format = tmp;

	memcpy(f->format, format, alloc_len);

	f->format = tmp;
	f->format_len = len;
	f->num_replacements = 0;
	f->num_percent = 0;

	/* Parse format string */
	for(size_t i = 0; i < len; i++) {
		if(format[i] == '%') {
			if(last_percent) {
				f->num_percent++;
				last_percent = 0;
			} else {
				last_percent = 1;
			}
		} else if(format[i] == 's' && last_percent) {
			f->num_replacements++;
			last_percent = 0;
		} else {
			last_percent = 0;
		}
	}

	return 0;
}

/* Processes a single input sample and returns the generated output sample. If
 * an error occurs, the function returns NULL. */
static char*
am_dfg_string_format_node_process_one_sample(struct am_dfg_string_format_node* f,
					     const char* in)
{
	size_t out_size;
	size_t in_len = strlen(in);
	int last_percent = 0;
	size_t out_pos = 0;
	char* out;

	/* Space for replaced text */
	if(am_size_mul_safe(&out_size, in_len, f->num_replacements))
		return NULL;

	/* Final '\0' */
	if(am_size_add_safe(&out_size, out_size, 1))
		return NULL;

	/* Subtraction is safe, since we're sure the characters are counted in
	 * f->format_len */
	if(am_size_add_safe(&out_size, out_size, f->format_len - f->num_percent))
		return NULL;

	if(!(out = malloc(out_size)))
		return NULL;

	/* Process format string */
	for(size_t i = 0; i < f->format_len; i++) {
		if(f->format[i] == '%') {
			if(last_percent) {
				last_percent = 0;
				out[out_pos++] = f->format[i];
			} else {
				last_percent = 1;
			}
		} else if(f->format[i] == 's' && last_percent) {
			last_percent = 0;
			memcpy(&out[out_pos], in, in_len);
			out_pos += in_len;
		} else {
			/* Copy any single percent sign verbatim */
			if(last_percent)
				out[out_pos++] = f->format[i];

			last_percent = 0;
			out[out_pos++] = f->format[i];
		}
	}

	out[out_pos] = '\0';

	return out;
}

int am_dfg_string_format_node_process(struct am_dfg_node* n)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	size_t old_num_samples;
	char** in;
	char* str;

	if(!am_dfg_port_is_connected(pin) || !am_dfg_port_is_connected(pout))
		return 0;

	old_num_samples = pout->buffer->num_samples;
	in = pin->buffer->data;

	/* Try to convert all samples. If one conversion fails, destroy samples
	 * produced so far in out_err_free. */
	for(size_t i = 0; i < pin->buffer->num_samples; i++) {
		if(!(str = am_dfg_string_format_node_process_one_sample(f, in[i])))
			goto out_err;

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


int am_dfg_string_format_node_set_property(struct am_dfg_node* n,
					   const struct am_dfg_property* property,
					   const void* value)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;
	char* const*  pstr = value;

	if(strcmp(property->name, "format") != 0)
		return 1;

	return am_dfg_string_format_node_set_format_string(f, *pstr);
}

int am_dfg_string_format_node_get_property(const struct am_dfg_node* n,
					   const struct am_dfg_property* property,
					   void** value)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;

	*value = f->format;

	return 0;
}

int am_dfg_string_format_node_from_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;
	const char* format;

	if(am_object_notation_eval_retrieve_string(
		   &g->node, "format", &format) == 0)
	{
		if(am_dfg_string_format_node_set_format_string(f, format))
			return 1;
	}

	return 0;
}

int am_dfg_string_format_node_to_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g)
{
	struct am_dfg_string_format_node* f = (typeof(f))n;
	struct am_object_notation_node_member* mformat;

	mformat = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "format",
			AM_OBJECT_NOTATION_BUILD_STRING, f->format);

	if(!mformat)
		return 1;

	am_object_notation_node_group_add_member(g, mformat);

	return 0;
}
