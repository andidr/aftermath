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

#ifndef AM_ON_DISK_H
#define AM_ON_DISK_H

#include <aftermath/core/convert.h>
#include <aftermath/core/trace.h>
#include <aftermath/core/io_context.h>
#include <aftermath/core/base_types.h>
#include <stdint.h>
#include <stdio.h>

{% set dsk = am_types.dsk %}

#define AM_TRACE_MAGIC 0x5654534f
#define AM_TRACE_VERSION 18

{% for tname in am_types.topological_sort(dsk.types) -%}
{% set t = dsk.types[tname] %}
{% if "type" in t.defs %}
{%- if t.comment -%}
/* {{t.comment}} */
{%- endif %}
{{t.c_type}} {
	{%- for field in t.fields %}
	{%- if field.comment %}
	/* {{field.comment}} */
	{%- endif %}
	{{dsk.find(field.type).c_type}} {{field.name}};
{# #}
{%- endfor -%}
} __attribute__((packed));
{% endif %}
{% endfor -%}

#define AM_DECL_ON_DISK_READ_INT_FUN(type, bits)				\
	static inline int am_dsk_read_##type(struct am_io_context* ctx,	\
					     type* out)			\
	{									\
		type tmp;							\
										\
		if(fread(&tmp, sizeof(tmp), 1, ctx->fp) != 1) {		\
			am_io_error_stack_push(&ctx->error_stack,		\
				AM_IOERR_READ,				\
				 "Could not read " #type " at offset %jd.",	\
				 ftello(ctx->fp));				\
			return 1;						\
		}								\
										\
		*out = am_int##bits##_letoh(tmp);					\
										\
		return 0;							\
	}

AM_DECL_ON_DISK_READ_INT_FUN(int16_t, 16)
AM_DECL_ON_DISK_READ_INT_FUN(uint16_t, 16)
AM_DECL_ON_DISK_READ_INT_FUN(int32_t, 32)
AM_DECL_ON_DISK_READ_INT_FUN(uint32_t, 32)
AM_DECL_ON_DISK_READ_INT_FUN(int64_t, 64)
AM_DECL_ON_DISK_READ_INT_FUN(uint64_t, 64)

static inline int am_dsk_read_uint8_t(struct am_io_context* ctx,
				      uint8_t* out)
{
	uint8_t tmp;

	if(fread(&tmp, sizeof(tmp), 1, ctx->fp) != 1) {
		am_io_error_stack_push(&ctx->error_stack,
					  AM_IOERR_READ,
					  "Could not read uint8_t at offset %jd.",
					  ftello(ctx->fp));
		return 1;
	}

	return 0;
}

#define AM_DECL_ON_DISK_WRITE_INT_FUN(type, bits)				\
	static inline int							\
	am_dsk_write_##type(struct am_io_context* ctx,				\
			    const type* in)					\
	{									\
		type tmp = am_int##bits##_htole(*in);				\
										\
		if(fwrite(&tmp, sizeof(tmp), 1, ctx->fp) != 1) {		\
			am_io_error_stack_push(&ctx->error_stack,		\
				AM_IOERR_WRITE,				\
				"Could not write " #type " at offset %jd.",	\
				 ftello(ctx->fp));				\
			return 1;						\
		}								\
										\
		return 0;							\
	}

AM_DECL_ON_DISK_WRITE_INT_FUN(int16_t, 16)
AM_DECL_ON_DISK_WRITE_INT_FUN(uint16_t, 16)
AM_DECL_ON_DISK_WRITE_INT_FUN(int32_t, 32)
AM_DECL_ON_DISK_WRITE_INT_FUN(uint32_t, 32)
AM_DECL_ON_DISK_WRITE_INT_FUN(int64_t, 64)
AM_DECL_ON_DISK_WRITE_INT_FUN(uint64_t, 64)

static inline int am_dsk_write_uint8_t(struct am_io_context* ctx,
				       const uint8_t* in)
{
	if(fwrite(&in, sizeof(*in), 1, ctx->fp) != 1) {
		am_io_error_stack_push(&ctx->error_stack,
					  AM_IOERR_WRITE,
					  "Could not write uint8_t at offset %jd.",
					  ftello(ctx->fp));
		return 1;
	}

	return 0;
}

{% for t in am_types.filter_list_hasdefs(dsk.types_list, ["dsk_write"]) -%}
{% include "dsk_write.tpl.fnproto.h" %}
{% endfor %}

int am_dsk_register_frame_types(struct am_frame_type_registry* r);
int am_dsk_load_trace(struct am_io_context* ctx, struct am_trace** pt);

#endif
