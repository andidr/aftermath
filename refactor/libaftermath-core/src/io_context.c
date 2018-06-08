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

#include <aftermath/core/io_context.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int am_io_context_init(struct am_io_context* ctx,
		       struct am_frame_type_registry* frame_types)
{
	ctx->trace = NULL;
	ctx->filename = NULL;
	ctx->fp = NULL;
	ctx->bounds_valid = 0;
	ctx->frame_types = frame_types;

	am_io_hierarchy_context_init(&ctx->hierarchy_context);
	am_io_index_to_id_maps_init(&ctx->index_to_id_maps);

	am_array_registry_init(&ctx->ecoll_associated_array_registry);

	am_event_collection_array_mapping_init(&ctx->ecoll_associated_arrays);

	if(am_io_error_stack_definit(&ctx->error_stack)) {
		am_event_collection_array_mapping_destroy(
			&ctx->ecoll_associated_arrays,
			&ctx->ecoll_associated_array_registry);
		return 1;
	}

	return 0;
}

void am_io_context_destroy(struct am_io_context* ctx)
{
	am_io_index_to_id_maps_destroy(&ctx->index_to_id_maps);
	am_event_collection_array_mapping_destroy(
		&ctx->ecoll_associated_arrays,
		&ctx->ecoll_associated_array_registry);
	am_io_context_reset(ctx);
	am_io_context_close(ctx);
	am_io_error_stack_destroy(&ctx->error_stack);
}

/* Sets the file name of an I/O context. The name is copied to a buffer owned by
 * the context and can safely be freed after the call. Returns 0 on success,
 * otherwise 1. */
int am_io_context_set_filename(struct am_io_context* ctx, const char* filename)
{
	char* tmp;

	if(filename) {
		if(!(tmp = strdup(filename)))
			return 1;
	} else {
		tmp = NULL;
	}

	free(ctx->filename);
	ctx->filename = tmp;

	return 0;
}

/* Opens a file in the specified mode m. If the operation fails, an error is
 * pushed onto the I/O error stack of the I/O context. Returns 0 on success,
 * otherwise 1.  */
int am_io_context_open(struct am_io_context* ctx,
		       const char* filename,
		       enum am_io_mode m)
{
	const char* fpmode;
	const char* fpmode_hr;
	enum am_io_error_id err;

	switch(m) {
		case AM_IO_READ:
			fpmode = "r";
			fpmode_hr = "reading";
			break;
		case AM_IO_WRITE:
			fpmode = "w+";
			fpmode_hr = "writing";
			break;
		default:
			AM_IOERR_RET1_NA(ctx,
					 AM_IOERR_UNKNOWN,
					 "Unknown I/O mode.");
	}

	if(!(ctx->fp = fopen(filename, fpmode))) {
		switch(errno) {
			case ENOENT:
				err = AM_IOERR_NOSUCHFILE;
				break;
			case EPERM:
				err = AM_IOERR_PERMISSION;
				break;
			default:
				err = AM_IOERR_UNKNOWN;
				break;
		}

		AM_IOERR_RET1(ctx,
			      err,
			      "Could not open trace file \"%s\" for %s.",
			      filename,
			      fpmode_hr);
	}

	if(am_io_context_set_filename(ctx, filename)) {
		fclose(ctx->fp);
		ctx->fp = NULL;
		return 1;
	}

	return 0;
}

/* Closes the file opened with the I/O context and resets the context's
 * filename. */
void am_io_context_close(struct am_io_context* ctx)
{
	if(ctx->fp) {
		fclose(ctx->fp);
		ctx->fp = NULL;
	}

	am_io_context_set_filename(ctx, NULL);
}

/* Resets an I/O context. If a trace has been associated with the context, the
 * trace is destroyed, but not freed. */
void am_io_context_reset(struct am_io_context* ctx)
{
	if(ctx->trace) {
		am_trace_destroy(ctx->trace);
		ctx->trace = NULL;
	}

	am_io_hierarchy_context_destroy(&ctx->hierarchy_context);
	am_io_hierarchy_context_init(&ctx->hierarchy_context);

	ctx->bounds_valid = 0;
}

/* Returns the array of a given type currently associated to the event
 * collection ecoll. If no such array is associated to the event collection, the
 * function returns NULL. */
void* am_io_context_find_event_collection_associated_array(
	struct am_io_context* ctx,
	struct am_event_collection* ecoll,
	const char* type)
{
	struct am_array_collection* ac;

	if(!(ac = am_event_collection_array_mapping_find(
		     &ctx->ecoll_associated_arrays, ecoll)))
	{
		return NULL;
	}

	return am_array_collection_find(ac, type);
}

/* Returns the array of a given type currently associated to the event
 * collection ecoll. If no such array is associated to the event collection, a
 * new array of the given type is created. If the creation fails, NULL is
 * returned. */
void* am_io_context_find_or_add_event_collection_associated_array(
	struct am_io_context* ctx,
	struct am_event_collection* ecoll,
	const char* type)
{
	void* ret;
	struct am_array_collection* ac;

	if(!(ac = am_event_collection_array_mapping_find_or_add(
		     &ctx->ecoll_associated_arrays, ecoll)))
	{
		return NULL;
	}

	if((ret = am_array_collection_find(ac, type)))
		return ret;

	if(!(ret = am_array_registry_allocate_and_init_array(
		     &ctx->ecoll_associated_array_registry,
		     type, NULL)))
	{
		return NULL;
	}

	if(am_array_collection_add(ac, ret, type)) {
		am_array_registry_destroy_and_free_array(
			&ctx->ecoll_associated_array_registry,
			type, NULL, ret);

		return NULL;
	}

	return ret;
}

void am_io_fail(void)
{
	/* Only for debugging; you may set a breakpoint here */
}
