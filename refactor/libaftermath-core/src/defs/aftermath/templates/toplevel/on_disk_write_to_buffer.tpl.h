/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * ************************************************************************
 * * THIS FILE IS PART OF THE CODE RELEASED UNDER THE LGPL, VERSION 2.1   *
 * * UNLIKE THE MAJORITY OF THE CODE OF LIBAFTERMATH-CORE, RELEASED UNDER *
 * * THE GPL, VERSION 2.                                                  *
 * ************************************************************************
 *
 * This file can be redistributed it and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_ON_DISK_WRITE_TO_BUFFER_H
#define AM_ON_DISK_WRITE_TO_BUFFER_H

/* Do not use system header-style include (i.e., <aftermath/core/...>, since
 * this file is shipped with other libraries */
#include "convert.h"
#include "base_types.h"
#include "on_disk_structs.h"
#include "on_disk_default_type_ids.h"
#include "write_buffer.h"
#include <string.h>

{% set dsk_types = aftermath.config.getDskTypes() %}

static inline int
am_dsk_uint8_t_write_to_buffer(struct am_write_buffer* wb, const uint8_t* in)
{
	uint8_t* dst;

	if(!(dst = (uint8_t*)am_write_buffer_reserve_bytes(
		     wb, sizeof(*in))))
	{
		return 1;
	}

	*dst = *in;

	return 0;
}

static inline int
am_dsk_float_write_to_buffer(struct am_write_buffer* wb, const float* in)
{
	float* dst;

	if(!(dst = (float*)am_write_buffer_reserve_bytes(
		     wb, sizeof(*in))))
	{
		return 1;
	}

	*dst = am_float32_htole(*in);

	return 0;
}

static inline int
am_dsk_double_write_to_buffer(struct am_write_buffer* wb, const double* in)
{
	double* dst;

	if(!(dst = (double*)am_write_buffer_reserve_bytes(
		     wb, sizeof(*in))))
	{
		return 1;
	}

	*dst = am_double64_htole(*in);

	return 0;
}

#define AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(type, bits)			\
	static inline int							\
	am_dsk_##type##_write_to_buffer(struct am_write_buffer* wb,		\
			      const type* in)					\
	{									\
		type* dst;							\
										\
		if(!(dst = (type*)am_write_buffer_reserve_bytes(		\
			     wb, sizeof(*in))))				\
		{								\
			return 1;						\
		}								\
										\
		*dst = am_int##bits##_htole(*in);				\
										\
		return 0;							\
	}

AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(int16_t, 16)
AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(uint16_t, 16)
AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(int32_t, 32)
AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(uint32_t, 32)
AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(int64_t, 64)
AM_DECL_ON_DISK_WRITE_TO_BUFFER_INT_FUN(uint64_t, 64)

/* Converts s into its final on-disk representation and writes the result into
 * the write buffer wb.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int am_dsk_string_write_to_buffer(struct am_write_buffer* wb,
						const struct am_dsk_string* s)
{
	uint32_t len = strlen(s->str);

	if(am_dsk_uint32_t_write_to_buffer(wb, &len))
		return 1;

	if(am_write_buffer_write_bytes(wb, len, s->str))
		return 1;

	return 0;
}

{% for t in dsk_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.dsk.GenerateWriteToBufferFunction) -%}
{{ tag.instantiateTemplate().getPrototype() }}
{% endfor -%}
{% endfor -%}
{# #}

{% for t in dsk_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.dsk.GenerateWriteToBufferFunction) -%}
{{ tag.instantiateTemplate() }}

{% endfor -%}
{% endfor -%}

{% for t in dsk_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.dsk.GenerateWriteToBufferWithDefaultIDFunction) -%}
{{ tag.instantiateTemplate() }}

{% endfor -%}
{% endfor -%}

/* Writes a frame type id frame to the write buffer wb, associating the type
 * of the specified name with the given id.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int am_dsk_write_default_id_to_buffer(struct am_write_buffer* wb,
						    char* name,
						    uint32_t id)
{
	struct am_dsk_frame_type_id fti;

	fti.id = id;
	fti.type_name.str = name;
	fti.type_name.len = strlen(name);

	return am_dsk_frame_type_id_write_to_buffer(wb, &fti);
}

{% for t in dsk_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.dsk.GenerateWriteDefaultIDToBufferFunction) -%}
{{ tag.instantiateTemplate() }}

{% endfor -%}
{% endfor -%}

#endif
