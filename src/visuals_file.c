/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "visuals_file.h"
#include "convert.h"

int visuals_header_conversion_table[] = {
	FIELD_SIZE(struct visuals_header, magic),
	FIELD_SIZE(struct visuals_header, version)
	CONVERSION_TABLE_END
};

#define VISUALS_ELEMENT_HEADER_CONVERSION_FIELDS \
	FIELD_SIZE(struct visuals_element_header, type)

int visuals_element_header_conversion_table[] = {
	VISUALS_ELEMENT_HEADER_CONVERSION_FIELDS,
	CONVERSION_TABLE_END
};

int visuals_annotation_conversion_table[] = {
	VISUALS_ELEMENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct visuals_annotation, cpu),
	FIELD_SIZE(struct visuals_annotation, time),
	FIELD_SIZE(struct visuals_annotation, length),
	CONVERSION_TABLE_END
};

int visuals_verify_header(struct visuals_header* header)
{
	return (header->magic == VISUALS_MAGIC &&
		header->version == VISUALS_VERSION);
}

int read_visual_elements(struct multi_event_set* mes, FILE* fp)
{
	struct visuals_element_header dsk_vh;
	struct visuals_annotation dsk_a;
	struct annotation a;
	int last_buffer_length = 0;
	int curr_buffer_length;
	char* buffer = NULL;
	int ret = 1;

	while(!feof(fp)) {
		if(read_uint32_convert(fp, &dsk_vh.type) != 0) {
			if(feof(fp))
				goto out_success;
			else
				goto out_err;
		}

		if(dsk_vh.type == VISUAL_TYPE_ANNOTATION) {
			memcpy(&dsk_a, &dsk_vh, sizeof(dsk_vh));
			if(read_struct_convert(fp, &dsk_a, sizeof(dsk_a), visuals_annotation_conversion_table, sizeof(dsk_vh)) != 0)
				goto out_err;

			a.cpu = dsk_a.cpu;
			a.time = dsk_a.time;

			curr_buffer_length = dsk_a.length+1;

			if(curr_buffer_length > last_buffer_length)
				if(!(buffer = realloc(buffer, curr_buffer_length)))
					goto out_err;

			if(fread(buffer, curr_buffer_length-1, 1, fp) != 1)
				goto out_err;

			buffer[curr_buffer_length-1] = '\0';

			if(annotation_init(&a, dsk_a.cpu, dsk_a.time, buffer))
				goto out_err;

			struct event_set* es = multi_event_set_find_cpu(mes, a.cpu);

			if(!es || event_set_add_annotation(es, &a)) {
				annotation_destroy(&a);
				goto out_err;
			}

			annotation_destroy(&a);
		}
	}

out_success:
	ret = 0;

out_err:
	if(buffer)
		free(buffer);

	return ret;
}

int load_visuals(const char* file, struct multi_event_set* mes)
{
	struct visuals_header header;

	int res = 1;
	FILE* fp;

	if(!(fp = fopen(file, "r")))
	   goto out;

	if(read_struct_convert(fp, &header, sizeof(header), trace_header_conversion_table, 0) != 0)
		goto out_fp;

	if(!visuals_verify_header(&header))
		goto out_fp;

	if(read_visual_elements(mes, fp) != 0)
		goto out_fp;

	res = 0;

out_fp:
	fclose(fp);
out:
	return res;
}

int write_visual_elements(struct multi_event_set* mes, FILE* fp)
{
	struct visuals_annotation dsk_a;

	for(struct event_set* es = mes->sets; es < &mes->sets[mes->num_sets]; es++) {
		for(struct annotation* a = es->annotations; a < &es->annotations[es->num_annotations]; a++) {
			dsk_a.header.type = VISUAL_TYPE_ANNOTATION;
			dsk_a.cpu = a->cpu;
			dsk_a.time = a->time;
			dsk_a.length = strlen(a->text);

			if(write_struct_convert(fp, &dsk_a, sizeof(dsk_a), visuals_annotation_conversion_table, 0) != 0)
				return 1;

			if(fwrite(a->text, strlen(a->text), 1, fp) != 1)
				return 1;
		}
	}

	return 0;
}

int store_visuals(const char* file, struct multi_event_set* mes)
{
	struct visuals_header header;

	header.magic = VISUALS_MAGIC;
	header.version = VISUALS_VERSION;

	int res = 1;
	FILE* fp;

	if(!(fp = fopen(file, "w+")))
	   goto out;

	if(write_struct_convert(fp, &header, sizeof(header), trace_header_conversion_table, 0) != 0)
		goto out_fp;

	if(write_visual_elements(mes, fp) != 0)
		goto out_fp;

	res = 0;

out_fp:
	fclose(fp);
out:
	return res;
}
