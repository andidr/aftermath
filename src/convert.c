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

#include "convert.h"
#include "ansi_extras.h"

/* Convert struct from disk format to host format */
void convert_struct(void* ptr, int* conversion_table, int offset, enum conversion_direction dir)
{
	int size;
	int i = 0;
	int curr_offset = 0;

	/* Skip fields at the beginning of the structure */
	while(curr_offset < offset) {
		curr_offset += conversion_table[i];
		i++;
	}

	ptr = ((char*)ptr)+offset;

	/* Convert remaining fields */
	while((size = conversion_table[i]) != CONVERSION_TABLE_END) {
		if(dir == CONVERT_DSK_TO_HOST) {
			switch(size) {
				case 2: *((uint16_t*)ptr) = int16_letoh(*((uint16_t*)ptr)); break;
				case 4: *((uint32_t*)ptr) = int32_letoh(*((uint32_t*)ptr)); break;
				case 8: *((uint64_t*)ptr) = int64_letoh(*((uint64_t*)ptr)); break;
			}
		} else {
			switch(size) {
				case 2: *((uint16_t*)ptr) = int16_htole(*((uint16_t*)ptr)); break;
				case 4: *((uint32_t*)ptr) = int32_htole(*((uint32_t*)ptr)); break;
				case 8: *((uint64_t*)ptr) = int64_htole(*((uint64_t*)ptr)); break;
			}
		}

		ptr = ((char*)ptr)+size;
		i++;
	}
}

#define READ_STRUCT(fp, st) fread(st, sizeof(*(st)), 1, fp)

int read_uint32_convert(FILE* fp, uint32_t* out)
{
	if(fread(out, sizeof(*out), 1, fp) != 1)
		return 1;

	*out = int32_letoh(*out);

	return 0;
}

int read_struct_convert(FILE* fp, void* out, int size, int* conversion_table, int offset)
{
	if(fread(((char*)out)+offset, size-offset, 1, fp) != 1)
		return 1;

	convert_struct(((char*)out)+offset, conversion_table, offset, CONVERT_DSK_TO_HOST);
	return 0;
}

int write_struct_convert(FILE* fp, void* in, int size, int* conversion_table, int offset)
{
	convert_struct(((char*)in)+offset, conversion_table, offset, CONVERT_HOST_TO_DSK);

	if(fwrite(((char*)in)+offset, size-offset, 1, fp) != 1)
		return 1;

	return 0;
}
