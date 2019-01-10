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
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/on_disk_structs.h>
#include <stdint.h>
#include <stdio.h>

static inline int am_dsk_float_read_fp_noconv(FILE* fp, float* out)
{
	if(fread(out, sizeof(*out), 1, fp) != 1)
		return 1;

	return 0;
}

static inline int am_dsk_float_read_fp(FILE* fp, float* out)
{
	if(am_dsk_float_read_fp_noconv(fp, out))
		return 1;

	*out = am_float32_letoh(*out);

	return 0;
}

static inline int am_dsk_float_read(struct am_io_context* ctx,
				    float* out)
{
	if(am_dsk_float_read_fp(ctx->fp, out)) {
		am_io_error_stack_push(&ctx->error_stack,
				       AM_IOERR_READ,
				       "Could not read float at offset %jd.",
				       ftello(ctx->fp));

		return 1;
	}

	return 0;
}

static inline int am_dsk_double_read_fp_noconv(FILE* fp, double* out)
{
	if(fread(out, sizeof(*out), 1, fp) != 1)
		return 1;

	return 0;
}

static inline int am_dsk_double_read_fp(FILE* fp, double* out)
{
	if(am_dsk_double_read_fp_noconv(fp, out))
		return 1;

	*out = am_double64_letoh(*out);

	return 0;
}

static inline int am_dsk_double_read(struct am_io_context* ctx,
				     double* out)
{
	if(am_dsk_double_read_fp(ctx->fp, out)) {
		am_io_error_stack_push(&ctx->error_stack,
				       AM_IOERR_READ,
				       "Could not read double at offset %jd.",
				       ftello(ctx->fp));

		return 1;
	}

	return 0;
}

#define AM_DECL_ON_DISK_READ_INT_FP_NOCONV_FUN(type)			\
	static inline int am_dsk_##type##_read_fp_noconv(FILE* fp, type* out)	\
	{									\
		if(fread(out, sizeof(*out), 1, fp) != 1)			\
			return 1;						\
										\
		return 0;							\
	}

#define AM_DECL_ON_DISK_READ_INT_FP_FUN(type, bits)				\
	AM_DECL_ON_DISK_READ_INT_FP_NOCONV_FUN(type)				\
										\
	static inline int am_dsk_##type##_read_fp(FILE* fp, type* out)		\
	{									\
		if(am_dsk_##type##_read_fp_noconv(fp, out))			\
			return 1;						\
										\
		*out = am_int##bits##_letoh(*out);				\
										\
		return 0;							\
	}

#define AM_DECL_ON_DISK_READ_INT_FUN(type, bits)				\
	static inline int am_dsk_##type##_read(struct am_io_context* ctx,	\
						   type* out)			\
	{									\
		if(am_dsk_##type##_read_fp(ctx->fp, out)) {			\
			am_io_error_stack_push(&ctx->error_stack,		\
				AM_IOERR_READ,					\
				 "Could not read " #type " at offset %jd.",	\
				 ftello(ctx->fp));				\
										\
			return 1;						\
		}								\
										\
		return 0;							\
	}

AM_DECL_ON_DISK_READ_INT_FP_NOCONV_FUN(int8_t)
#define am_dsk_int8_t_read_fp am_dsk_int8_t_read_fp_noconv
AM_DECL_ON_DISK_READ_INT_FP_NOCONV_FUN(uint8_t)
#define am_dsk_uint8_t_read_fp am_dsk_uint8_t_read_fp_noconv
AM_DECL_ON_DISK_READ_INT_FP_FUN(int16_t, 16)
AM_DECL_ON_DISK_READ_INT_FP_FUN(uint16_t, 16)
AM_DECL_ON_DISK_READ_INT_FP_FUN(int32_t, 32)
AM_DECL_ON_DISK_READ_INT_FP_FUN(uint32_t, 32)
AM_DECL_ON_DISK_READ_INT_FP_FUN(int64_t, 64)
AM_DECL_ON_DISK_READ_INT_FP_FUN(uint64_t, 64)

AM_DECL_ON_DISK_READ_INT_FUN(int8_t, 8)
AM_DECL_ON_DISK_READ_INT_FUN(uint8_t, 8)
AM_DECL_ON_DISK_READ_INT_FUN(int16_t, 16)
AM_DECL_ON_DISK_READ_INT_FUN(uint16_t, 16)
AM_DECL_ON_DISK_READ_INT_FUN(int32_t, 32)
AM_DECL_ON_DISK_READ_INT_FUN(uint32_t, 32)
AM_DECL_ON_DISK_READ_INT_FUN(int64_t, 64)
AM_DECL_ON_DISK_READ_INT_FUN(uint64_t, 64)

#define AM_DECL_ON_DISK_DUMP_STDOUT_FUN(type, fmt)			\
	static inline int am_dsk_##type##_dump_stdout(			\
		struct am_io_context* ctx,				\
		type* d,						\
		size_t indent,						\
		size_t next_indent)					\
	{								\
		am_fprintf_prefix(stdout, "\t", indent, "%" fmt, *d);	\
		return 0;						\
	}

AM_DECL_ON_DISK_DUMP_STDOUT_FUN(char, "c")
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(int, "d")
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(float, "f")
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(double, "lf")
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(size_t, "zu")
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(int8_t, PRId8)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(uint8_t, PRIu8)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(int16_t, PRId16)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(uint16_t, PRIu16)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(int32_t, PRId32)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(uint32_t, PRIu32)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(int64_t, PRId64)
AM_DECL_ON_DISK_DUMP_STDOUT_FUN(uint64_t, PRIu64)

static inline int
am_dsk_float_write_noconv(struct am_io_context* ctx, const float in)
{
	if(fwrite(&in, sizeof(in), 1, ctx->fp) != 1) {
		am_io_error_stack_push(&ctx->error_stack,
				       AM_IOERR_WRITE,
				       "Could not write float at offset %jd.",
				       ftello(ctx->fp));
		return 1;
	}

	return 0;
}

static inline int
am_dsk_float_write(struct am_io_context* ctx, const float* in)
{
	float tmp = am_float32_htole(*in);
	return am_dsk_float_write_noconv(ctx, tmp);
}

static inline int
am_dsk_double_write_noconv(struct am_io_context* ctx, const double in)
{
	if(fwrite(&in, sizeof(in), 1, ctx->fp) != 1) {
		am_io_error_stack_push(&ctx->error_stack,
				       AM_IOERR_WRITE,
				       "Could not write double at offset %jd.",
				       ftello(ctx->fp));
		return 1;
	}

	return 0;
}

static inline int
am_dsk_double_write(struct am_io_context* ctx, const double* in)
{
	double tmp = am_double64_htole(*in);
	return am_dsk_double_write_noconv(ctx, tmp);
}

#define AM_DECL_ON_DISK_WRITE_INT_NOCONV_FUN(type)				\
	static inline int							\
	am_dsk_##type##_write_noconv(struct am_io_context* ctx,		\
				     const type* out)				\
	{									\
		if(fwrite(out, sizeof(*out), 1, ctx->fp) != 1) {			\
			am_io_error_stack_push(&ctx->error_stack,		\
				AM_IOERR_WRITE,				\
				"Could not write " #type " at offset %jd.",	\
				 ftello(ctx->fp));				\
			return 1;						\
		}								\
										\
		return 0;							\
	}

#define AM_DECL_ON_DISK_WRITE_INT_FUN(type, bits)				\
	AM_DECL_ON_DISK_WRITE_INT_NOCONV_FUN(type)				\
										\
	static inline int							\
	am_dsk_##type##_write(struct am_io_context* ctx, const type* out)	\
	{									\
		type tmp = am_int##bits##_htole(*out);				\
		return am_dsk_##type##_write_noconv(ctx, &tmp);		\
	}

AM_DECL_ON_DISK_WRITE_INT_NOCONV_FUN(int8_t)
#define am_dsk_int8_t_write am_dsk_int8_t_write_noconv
AM_DECL_ON_DISK_WRITE_INT_NOCONV_FUN(uint8_t)
#define am_dsk_uint8_t_write am_dsk_uint8_t_write_noconv
AM_DECL_ON_DISK_WRITE_INT_FUN(int16_t, 16)
AM_DECL_ON_DISK_WRITE_INT_FUN(uint16_t, 16)
AM_DECL_ON_DISK_WRITE_INT_FUN(int32_t, 32)
AM_DECL_ON_DISK_WRITE_INT_FUN(uint32_t, 32)
AM_DECL_ON_DISK_WRITE_INT_FUN(int64_t, 64)
AM_DECL_ON_DISK_WRITE_INT_FUN(uint64_t, 64)

{% for t in aftermath.config.getDskTypes().filterByTag(aftermath.tags.dsk.GenerateWriteFunction) -%}
{{ aftermath.templates.dsk.WriteFunction(t).getPrototype() }}
{% endfor %}

{% for t in aftermath.config.getDskTypes().filterByTag(aftermath.tags.dsk.GenerateWriteDefaultIDFunction) -%}
{{ aftermath.templates.dsk.WriteDefaultIDFunction(t).getPrototype() }}
{% endfor %}

{% for t in aftermath.config.getDskTypes().filterByTag(aftermath.tags.dsk.GenerateWriteWithDefaultIDFunction) -%}
{{ aftermath.templates.dsk.WriteWithDefaultIDFunction(t).getPrototype() }}
{% endfor %}

int am_dsk_register_frame_types(struct am_frame_type_registry* r);
int am_dsk_load_trace(struct am_io_context* ctx, struct am_trace** pt);
int am_dsk_dump_trace(struct am_io_context* ctx,
		      const char* filename,
		      off_t start_offs,
		      off_t end_offs,
		      int dump_offsets);

#endif
