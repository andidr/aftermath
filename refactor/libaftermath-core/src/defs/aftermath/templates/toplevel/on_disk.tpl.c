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

#include <aftermath/core/counter_description_array.h>
#include <aftermath/core/measurement_interval_array.h>
#include <aftermath/core/openmp_for_loop_type_array.h>
#include <aftermath/core/openmp_for_loop_instance_array.h>
#include <aftermath/core/openmp_iteration_set_array.h>
#include <aftermath/core/openmp_iteration_period_array.h>
#include <aftermath/core/openmp_task_instance_array.h>
#include <aftermath/core/openmp_task_period_array.h>
#include <aftermath/core/openmp_task_type_array.h>
#include <aftermath/core/openstream_task_instance_array.h>
#include <aftermath/core/openstream_task_period_array.h>
#include <aftermath/core/openstream_task_type_array.h>
#include <aftermath/core/state_description_array.h>
#include <aftermath/core/state_event_array.h>
#include <aftermath/core/counter_event_array_collection.h>

#include <aftermath/core/telamon_candidate_array.h>

#include <aftermath/core/tensorflow_node_array.h>
#include <aftermath/core/tensorflow_node_execution_array.h>

#include <aftermath/core/on_disk.h>
#include <aftermath/core/on_disk_default_type_ids.h>
#include <aftermath/core/in_memory_inline.h>
#include <aftermath/core/on_disk_meta.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

{% set dsk_types = aftermath.config.getDskTypes() %}
{% set mem_types = aftermath.config.getMemTypes() %}
{% set meta_types = aftermath.config.getMetaTypes() %}

static int am_dsk_read_frames(struct am_io_context* ctx);

{% for t in dsk_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.TemplatedGenerateFunctionTag) -%}
{% if not isinstance(tag, aftermath.tags.dsk.WriteToBufferFunction) and
      not isinstance(tag, aftermath.tags.dsk.WriteToBufferWithDefaultIDFunction) and
      not isinstance(tag, aftermath.tags.dsk.WriteDefaultIDToBufferFunction) -%}
{{ tag.instantiateTemplate().getPrototype() }}
{% endif -%}
{% endfor -%}
{% endfor %}

/* Reads size bytes from the currently opened trace file. Returns 0 on success,
 * otherwise 1. Reads of size 0 always succeed. */
static inline int am_dsk_read(struct am_io_context* ctx,
			      void* buf,
			      size_t size)
{
	if(size == 0)
		return 0;

	if(fread(buf, size, 1, ctx->fp) != 1) {
		AM_IOERR_RET1(ctx, AM_IOERR_READ,
			      "Could not read %zu bytes at offset %jd.",
			      ftello(ctx->fp));
	}

	return 0;
}

static inline void* am_dsk_malloc(struct am_io_context* ctx, size_t size)
{
	void* tmp;

	if(!(tmp = malloc(size))) {
		am_io_error_stack_push(&ctx->error_stack,
					  AM_IOERR_ALLOC,
					  "Could not allocate %zu bytes.",
					  size);
	}

	return tmp;
}

/* Convert a non-zero-terminated string to a zero-terminated string. Returns the
 * a newly allocated copy of the string or NULL on error. */
static inline char* am_dsk_string_to_stringz(struct am_io_context* ctx,
					     const struct am_dsk_string* str)
{
	char* ret;

	if(!(ret = am_dsk_malloc(ctx, str->len+1)))
		return NULL;

	strncpy(ret, str->str, str->len);
	ret[str->len] = '\0';

	return ret;
}

static inline int am_dsk_string_to_mem(struct am_io_context* ctx,
				       struct am_dsk_string* dsk,
				       char** out)
{
	char* s;

	if(!(s = am_dsk_string_to_stringz(ctx, dsk)))
		return 1;

	*out = s;

	return 0;
}

{% for t in dsk_types.filterByTag(aftermath.tags.dsk.tomem.GenerateConversionFunction) %}
{{ aftermath.templates.dsk.tomem.ConversionFunction(t) }}
{%- endfor %}

/* Write size bytes to the currently opened trace file. Returns 0 on success,
 * otherwise 1. Writes of size 0 always succeed. */
static inline int am_dsk_write(struct am_io_context* ctx,
			       void* buf,
			       size_t size)
{
	if(size == 0)
		return 0;

	if(fwrite(buf, size, 1, ctx->fp) != 1) {
		AM_IOERR_RET1(ctx, AM_IOERR_WRITE,
			      "Could not write %zu bytes at offset %jd.",
			      ftello(ctx->fp));
	}

	return 0;
}

static inline int am_dsk_string_read(struct am_io_context* ctx,
				     struct am_dsk_string* s)
{
	if(am_dsk_uint32_t_read(ctx, &s->len)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_READ,
				 "Could not read string length.");
	}

	if(!(s->str = am_dsk_malloc(ctx, s->len))) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_READ,
				 "Could not allocate string.");
	}

	if(am_dsk_read(ctx, s->str, s->len)) {
		free(s->str);
		AM_IOERR_RET1_NA(ctx, AM_IOERR_READ,
				 "Could not read string.");
	}

	return 0;
}

int am_dsk_string_write(struct am_io_context* ctx, const struct am_dsk_string* s)
{
	if(am_dsk_uint32_t_write(ctx, &s->len)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_WRITE,
				 "Could not write string length.");
	}

	if(am_dsk_write(ctx, s->str, s->len)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_WRITE,
				 "Could not write string.");
	}

	return 0;
}

static inline int am_dsk_header_assert(struct am_io_context* ctx,
				       const struct am_dsk_header* f)
{
	if(f->magic != AM_TRACE_MAGIC)
		AM_IOERR_RET1_NA(ctx, AM_IOERR_MAGIC, "Wrong file type.");

	if(f->version != AM_TRACE_VERSION) {
		AM_IOERR_RET1(ctx, AM_IOERR_VERSION,
			      "Wrong file version: expected %"PRIu64", but got "
			      "%" PRIu64 ".", AM_TRACE_VERSION, f->version);
	}

	return 0;
}

static inline int am_dsk_state_event_assert(struct am_io_context* ctx,
					    const struct am_dsk_state_event* f)
{
	if(am_dsk_interval_assert(ctx, &f->interval))
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ASSERT, "Invalid state frame.");

	return 0;
}

/* Associates the frame type specified in the frame type ID association
 * structure fti with the specified ID in the context's frame type registry.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int
am_dsk_frame_type_id_process(struct am_io_context* ctx,
			     struct am_dsk_frame_type_id* fti)
{
	struct am_frame_type* ft;
	size_t id_size;
	char* namez;
	int ret = 1;

	if(!(namez = am_dsk_string_to_stringz(ctx, &fti->type_name))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ASSERT,
				 "Could not convert string for type name.");
	}

	if(!(ft = am_frame_type_registry_find(ctx->frame_types, namez))) {
		AM_IOERR_GOTO(ctx, out_err_free, AM_IOERR_FIND_RELATED,
			      "Could not find type \"%s\".", namez);
	}

	if(fti->id == 0) {
		AM_IOERR_GOTO(ctx, out_err_free, AM_IOERR_ASSERT,
			      "Invalid value %" PRIu32 " for a type ID, value "
			      "0 is reserved.",
			      fti->id);
	}

	if(am_safe_size_from_u32(&id_size, fti->id)) {
		AM_IOERR_GOTO(ctx, out_err_free, AM_IOERR_ASSERT,
			      "Invalid value %" PRIu32 " for a type ID.",
			      fti->id);
	}

	if(am_frame_type_registry_set_id(ctx->frame_types, ft, id_size)) {
		AM_IOERR_GOTO(ctx, out_err_free, AM_IOERR_ASSERT,
			      "Could not associate ID %zu with type \"%s\".",
			      id_size, namez);
	}

	ret = 0;

out_err_free:
	free(namez);
out_err:
	return ret;

}

static inline int
am_dsk_hierarchy_description_process(struct am_io_context* ctx,
				     struct am_dsk_hierarchy_description* f)
{
	struct am_trace* t = ctx->trace;
	char* namez;
	struct am_hierarchy* h;

	if(am_io_hierarchy_context_find_hierarchy(&ctx->hierarchy_context, f->id)) {
		AM_IOERR_GOTO(ctx, out, AM_IOERR_ASSERT,
			      "Hierarchy with ID %" AM_HIERARCHY_ID_T_FMT " already defined.",
			      f->id);
	}

	if(!(namez = am_dsk_string_to_stringz(ctx, &f->name))) {
		AM_IOERR_GOTO_NA(ctx, out, AM_IOERR_CONVERT,
				 "Could not convert string for hierarchy name.");
	}

	if(!(h = am_dsk_malloc(ctx, sizeof(*h)))) {
		AM_IOERR_GOTO_NA(ctx, out_free_namez, AM_IOERR_ALLOC,
				 "Could not allocate memory for hierarchy.");
	}

	am_hierarchy_init_nodup(h, namez, f->id);

	if(am_hierarchyp_array_add(&t->hierarchies, h)) {
		AM_IOERR_GOTO_NA(ctx, out_destroy, AM_IOERR_ADD,
				 "Could not add hierarchy.");
	}

	if(am_io_hierarchy_context_add_hierarchy(&ctx->hierarchy_context, h, f->id)) {
		AM_IOERR_GOTO_NA(ctx, out_remove, AM_IOERR_ADD,
				 "Could not add hierarchy to hierarchy IO context.");
	}

	return 0;

out_remove:
	am_hierarchyp_array_remove_sorted(&t->hierarchies, h->id);
out_destroy:
	am_hierarchy_destroy(h);
	return 1;

out_free_namez:
	free(namez);
out:
	return 1;
}

static inline int
am_dsk_hierarchy_node_process(struct am_io_context* ctx,
			      struct am_dsk_hierarchy_node* f)
{
	struct am_trace* t = ctx->trace;
	struct am_hierarchy* h;
	struct am_hierarchy** hp;
	struct am_hierarchy_node* hn;
	struct am_hierarchy_node* hn_parent = NULL;
	char* namez;

	if(!(hp = am_hierarchyp_array_find(&t->hierarchies, f->hierarchy_id))) {
		AM_IOERR_GOTO(ctx, out, AM_IOERR_ASSERT,
			      "Unknown hierarchy with ID %" AM_HIERARCHY_ID_T_FMT ".",
			      f->hierarchy_id);
	}

	h = *hp;

	/* If id == 0 -> Node is the root node of the hierarchy */
	if(f->parent_id == 0) {
		if(h->root) {
			AM_IOERR_GOTO(ctx, out, AM_IOERR_ASSERT,
				      "Hierarchy with ID %" AM_HIERARCHY_ID_T_FMT " "
				      "already has a root (offending node id is %" AM_HIERARCHY_NODE_ID_T_FMT ").",
				      f->hierarchy_id, f->id);
		}
	} else {
		/* Node is a regular node */
		if(am_io_hierarchy_context_find_hierarchy_node(&ctx->hierarchy_context, f->hierarchy_id, f->id)) {
			AM_IOERR_GOTO(ctx, out, AM_IOERR_ASSERT,
				      "Hierarchy node with ID %" AM_HIERARCHY_NODE_ID_T_FMT " "
				      "already declared for hierarchy ID %" AM_HIERARCHY_ID_T_FMT ".",
				      f->id, f->hierarchy_id);
		}

		if(!(hn_parent = am_io_hierarchy_context_find_hierarchy_node(&ctx->hierarchy_context, f->hierarchy_id, f->parent_id))) {
			AM_IOERR_GOTO(ctx, out, AM_IOERR_ASSERT,
				      "Unknown parent with ID %" AM_HIERARCHY_NODE_ID_T_FMT " "
				      "for Hierarchy node with ID %" AM_HIERARCHY_NODE_ID_T_FMT " "
				      "of hierarchy ID %" AM_HIERARCHY_ID_T_FMT ".",
				      f->parent_id, f->id, f->hierarchy_id);
		}
	}

	if(!(namez = am_dsk_string_to_stringz(ctx, &f->name))) {
		AM_IOERR_GOTO_NA(ctx, out, AM_IOERR_CONVERT,
				 "Could not convert hierarchy node name.");
	}

	if(!(hn = am_dsk_malloc(ctx, sizeof(*hn)))) {
		AM_IOERR_GOTO_NA(ctx, out_name, AM_IOERR_ALLOC,
				 "Could not allocate hierarchy node.");
	}

	am_hierarchy_node_init_nodup(hn, f->id, namez);

	if(am_io_hierarchy_context_add_hierarchy_node(&ctx->hierarchy_context,
						   f->hierarchy_id,
						   hn,
						   f->id))
	{
		AM_IOERR_GOTO_NA(ctx, out_destroy, AM_IOERR_ADD,
				 "Could not add hierarchy node to hierarchy context.");
	}

	if(f->parent_id == 0)
		h->root = hn;
	else
		am_hierarchy_node_add_child(hn_parent, hn);

	return 0;

out_destroy:
	am_hierarchy_node_destroy(hn);
	free(hn);
	return 1;
out_name:
	free(namez);
out:
	return 1;
}

static inline int am_dsk_event_collection_process(struct am_io_context* ctx,
						  struct am_dsk_event_collection* e)
{
	struct am_event_collection* emem;
	char* namez;

	if(am_event_collection_array_find(&ctx->trace->event_collections, e->id)) {
		AM_IOERR_GOTO(ctx, out_err, AM_IOERR_DUPLICATE,
			      "Event collection with ID %" AM_EVENT_COLLECTION_ID_T_FMT " "
			      "already defined.", e->id);
	}

	if(!(namez = am_dsk_string_to_stringz(ctx, &e->name))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_CONVERT,
				 "Could not convert string for event collection.");
	}

	if(!(emem = am_event_collection_array_add_nodup(&ctx->trace->event_collections, e->id, namez))) {
		AM_IOERR_GOTO_NA(ctx, out_err_namez, AM_IOERR_ADD,
				 "Could not add event collection.");
	}

	return 0;

out_err_namez:
	free(namez);
out_err:
	return 1;
}

static inline int am_dsk_event_mapping_process(struct am_io_context* ctx,
					       struct am_dsk_event_mapping* m)
{
	struct am_hierarchy_node* hn;
	struct am_trace* t = ctx->trace;
	struct am_event_collection* e;
	struct am_interval mem_interval;

	if(!(e = am_event_collection_array_find(&t->event_collections, m->collection_id))) {
		AM_IOERR_RET1(ctx, AM_IOERR_FIND_RELATED,
			      "Could not find event collection with ID %" AM_EVENT_COLLECTION_ID_T_FMT ".",
			      m->collection_id);
	}

	if(!(hn = am_io_hierarchy_context_find_hierarchy_node(&ctx->hierarchy_context, m->hierarchy_id, m->node_id))) {
		/* Something went wrong. Find out if the hierarchy does not
		 * exist or if there is no node with that ID in that
		 * hierarchy. */
		if(!am_io_hierarchy_context_find_hierarchy(&ctx->hierarchy_context, m->hierarchy_id)) {
			AM_IOERR_RET1(ctx, AM_IOERR_FIND_RELATED,
				      "Unknown hierarchy with ID %" AM_HIERARCHY_ID_T_FMT ".",
				      m->hierarchy_id);
		} else {
			AM_IOERR_RET1(ctx, AM_IOERR_FIND_RELATED,
				      "Unknown node with ID %" AM_HIERARCHY_NODE_ID_T_FMT " "
				      "for hierarchy with ID %" AM_HIERARCHY_ID_T_FMT ".",
				      m->node_id,
				      m->hierarchy_id);
		}
	}

	/* Convert on-disk data structure for interval to in-memory type */
	mem_interval.start = m->interval.start;
	mem_interval.end = m->interval.end;

	if(am_event_mapping_append_safe(&hn->event_mapping, &mem_interval, e)) {
		AM_IOERR_RET1(ctx, AM_IOERR_FIND_RELATED,
			      "Unknown node with ID %" AM_HIERARCHY_NODE_ID_T_FMT " "
			      "for hierarchy with ID %" AM_HIERARCHY_ID_T_FMT ".",
			      m->node_id,
			      m->hierarchy_id);
	}

	return 0;
}

/* Dumps the contents of a am_dsk_string to stdout.  The parameter
 * indent indicates by hiow many tabs the first line should be indented, while
 * next_indent indicates by how many tabs the following lines belonging to this
 * structure should be indicated.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int am_dsk_string_dump_stdout(struct am_io_context* ctx,
					    struct am_dsk_string* dsk,
					    size_t indent,
					    size_t next_indent)
{
	char* stringz;
	char* escaped;
	int ret = 1;

	if(!(stringz = am_dsk_string_to_stringz(ctx, dsk)))
		goto out;

	if(!(escaped = am_escape_string(stringz)))
		goto out_stringz;

	am_fprintf_prefix(stdout, "\t", indent, "am_dsk_string {\n");
	am_fprintf_prefix(stdout, "\t", next_indent+1, "len: ");
	am_dsk_uint32_t_dump_stdout(ctx, &dsk->len, 0, next_indent+1);
	printf(",\n");
	am_fprintf_prefix(stdout, "\t", next_indent+1, "str: \"%s\"\n", escaped);
	am_fprintf_prefix(stdout, "\t", next_indent, "}");

	ret = 0;

	free(escaped);

out_stringz:
	free(stringz);
out:
	return ret;
}

/* Reads and processes all frames of a trace file. The file pointer of the I/O
 * context must be positioned at the beginning of the first frame to read (i.e.,
 * the file header must have been skipped). Returns 0 on success, otherwise 1.
 */
static int am_dsk_read_frames(struct am_io_context* ctx)
{
	uint32_t type_id;
	size_t type_id_size;
	struct am_frame_type* ft;

	while(!feof(ctx->fp)) {
		if(am_dsk_uint32_t_read_fp(ctx->fp, &type_id)) {
			if(feof(ctx->fp)) {
				return 0;
			} else {
				AM_IOERR_RET1_NA(
					ctx, AM_IOERR_CONVERT,
					"Could not read frame type ID.");
			}
		}

		if(am_safe_size_from_u32(&type_id_size, type_id)) {
			AM_IOERR_RET1(ctx, AM_IOERR_CONVERT,
				      "Could not convert frame type ID "
				      "%" PRIu32 ".",
				      type_id);
		}

		if(!(ft = am_frame_type_registry_by_id(ctx->frame_types,
						       type_id_size)))
		{
			AM_IOERR_RET1(ctx, AM_IOERR_FIND_RELATED,
				      "Could not find frame type with ID "
				      "%zu.",
				      type_id_size);
		}

		if(ft->load) {
			if(ft->load(ctx)) {
				AM_IOERR_RET1(ctx, AM_IOERR_LOAD_FRAME,
					      "Could not load frame of type "
					      "%s.",
					      ft->name);
			}
		}
	}

	return 0;
}

static int am_dsk_header_verify(struct am_io_context* ctx)
{
	struct am_dsk_header h;

	if(am_dsk_header_read(ctx, &h))
		return 1;

	if(am_dsk_header_assert(ctx, &h))
		return 1;

	return 0;
}

/* Performs postprocessing of a trace after loading it into memory. Returns 0 on
 * success, otherwise 1. */
int am_dsk_postprocess(struct am_io_context* ctx)
{
	{%- for memtype in mem_types.filterByTag(aftermath.tags.postprocess.PostprocessFunction) + meta_types.filterByTag(aftermath.tags.postprocess.PostprocessFunction) %}
	if({{memtype.getTagInheriting(aftermath.tags.postprocess.PostprocessFunction).getFunctionName()}}(ctx))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}

/* Performs finalization of a trace after loading it into memory and after
 * postprocessing. Returns 0 on success, otherwise 1. */
int am_dsk_finalize(struct am_io_context* ctx)
{
	{%- for memtype in mem_types.filterByTag(aftermath.tags.finalize.FinalizeFunction) %}
	if({{memtype.getTagInheriting(aftermath.tags.finalize.FinalizeFunction).getFunctionName()}}(ctx))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}

/* Performs postprocessing of a trace after loading it into memory, after
 * postprocessing and after finalization. Returns 0 on success, otherwise 1. */
int am_dsk_teardown(struct am_io_context* ctx)
{
	{%- for memtype in mem_types.filterByTag(aftermath.tags.teardown.TeardownFunction) %}
	if({{memtype.getTagInheriting(aftermath.tags.teardown.TeardownFunction).getFunctionName()}}(ctx))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}

/* Loads a trace from disk into memory. A pointer to the newly allocated trace
 * data structure is stored in *pt. Ctx is a pointer to an already initialized
 * I/O context. If an error occurs, the error stack of the I/O context is set
 * accordingly. If no error occurs, the I/O context is reset before
 * returning. Returns 0 on sucess, otherwise 1.
 */
int am_dsk_load_trace(struct am_io_context* ctx, struct am_trace** pt)
{
	struct am_trace* t;

	if(!(t = malloc(sizeof(*t)))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ALLOC,
				 "Could not allocate trace.");
	}

	if(am_trace_init(t, ctx->filename)) {
		AM_IOERR_GOTO_NA(ctx, out_err_trace_free, AM_IOERR_INIT,
				 "Could not initialize trace.");
	}

	ctx->trace = t;

	if(am_dsk_header_verify(ctx)) {
		AM_IOERR_GOTO_NA(ctx, out_err_trace_destroy, AM_IOERR_ASSERT,
				 "Invalid header.");
	}

	if(am_dsk_read_frames(ctx)) {
		AM_IOERR_GOTO_NA(ctx, out_err_trace_destroy, AM_IOERR_READ_FRAMES,
				 "Could not read frames.");
	}

	if(am_dsk_postprocess(ctx)) {
		AM_IOERR_GOTO_NA(ctx, out_err_trace_destroy,
				 AM_IOERR_POSTPROCESS, "Postprocessing failed.");
	}

	if(am_dsk_finalize(ctx)) {
		AM_IOERR_GOTO_NA(ctx, out_err_trace_destroy,
				 AM_IOERR_POSTPROCESS, "Finalization failed.");
	}

	if(am_dsk_teardown(ctx)) {
		AM_IOERR_GOTO_NA(ctx, out_err_trace_destroy,
				 AM_IOERR_POSTPROCESS, "Teardown step failed.");
	}

	*pt = ctx->trace;
	ctx->trace = NULL;

	am_io_context_reset(ctx);

	return 0;

out_err_trace_destroy:
	am_trace_destroy(t);
out_err_trace_free:
	free(t);
	ctx->trace = NULL;
out_err:
	am_io_context_reset(ctx);
	return 1;
}

/* Registers all builtin frame types at the frame type registry r. Returns 0 on
 * success, otherwise 1. */
int am_dsk_register_frame_types(struct am_frame_type_registry* r)
{
	struct am_frame_type* ft;

	{%- for t in dsk_types.filterByTag(aftermath.tags.dsk.Frame) %}
	{%- set read_tag = t.getTagInheriting(aftermath.tags.dsk.ReadFunction) -%}
	{%- set load_tag = t.getTagInheriting(aftermath.tags.dsk.LoadFunction) -%}
	{%- set destroy_tag = t.getTagInheriting(aftermath.tags.Destructor) -%}
	{%- set dump_tag = t.getTagInheriting(aftermath.tags.dsk.DumpStdoutFunction) -%}

	{%- if destroy_tag %}
	{%- set destroy_fun = "AM_FRAME_REGISTRY_DESTROY_FUN("+destroy_tag.getFunctionName()+")" %}
	{%- else %}
	{%- set destroy_fun = "NULL" %}
	{%- endif %}

	if(am_frame_type_registry_add(r,
				      "{{t.getName()}}",
				      sizeof({{t.getCType()}}),
				      {{load_tag.getFunctionName()}},
				      AM_FRAME_REGISTRY_READ_FUN({{read_tag.getFunctionName()}}),
				      {{destroy_fun}},
				      AM_FRAME_REGISTRY_DUMP_FUN({{dump_tag.getFunctionName()}})))
	{
		return 1;
	}
	{%- endfor %}

	if(!(ft = am_frame_type_registry_find(r, "am_dsk_frame_type_id")))
		return 1;

	/* Frame type ID frame always has ID 0 */
	if(am_frame_type_registry_set_id(r, ft, 0))
		return 1;

	return 0;
}

#define AM_MAX_DUMP_FRAME_TYPES 128

/* Dumps all frames of a trace file to stdout. The file pointer of the I/O
 * context must be positioned at the beginning of the first frame to read (i.e.,
 * the file header must have been skipped).
 *
 * start_offs indicates the position in the file from which on data structures
 * should be dumped (if different from 0 data structures are skipped).
 *
 * end_offs indicates at which file offset the dump should be stopped. If zero,
 * the entire file is dumped.
 *
 * Offsets are inclusive; If a frame covers a portion of the interval
 * [start_offs; end_offs], the frame is dumped to stdout.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dsk_dump_frames(struct am_io_context* ctx,
		       off_t start_offs,
		       off_t end_offs,
		       int dump_offsets)
{
	uint32_t type_id;
	size_t type_id_size;
	struct am_frame_type* ft;
	size_t highest_size = 0;
	void* frame = NULL;
	void* tmp;
	int ret = 1;
	int check_offsets = 0;
	off_t frame_start_offs = 0;
	off_t frame_end_offs;
	int do_dump;
	int is_first;

	if(start_offs > 0 || end_offs != 0)
		check_offsets = 1;

	while(!feof(ctx->fp)) {
		if(check_offsets || dump_offsets)
			frame_start_offs = ftello(ctx->fp);

		if(am_dsk_uint32_t_read(ctx, &type_id)) {
			if(feof(ctx->fp))
				return 0;
			else
				return 1;
		}

		if(am_safe_size_from_u32(&type_id_size, type_id)) {
			AM_IOERR_GOTO(ctx,
				      out_free,
				      AM_IOERR_CONVERT,
				      "Could not convert frame type ID "
				      "%" PRIu32 ".",
				      type_id);
		}

		if(check_offsets) {
			/* If the next frame is past the end offset,
			 * terminate */
			if(end_offs != 0 && frame_start_offs > end_offs) {
				ret = 0;
				goto out_free;
			}
		}

		if(!(ft = am_frame_type_registry_by_id(ctx->frame_types,
						       type_id_size)))
		{
			AM_IOERR_GOTO(ctx,
				      out_free,
				      AM_IOERR_FIND_RELATED,
				      "Could not find frame type with ID "
				      "%zu.",
				      type_id_size);
		}

		/* Be default, dump the frame */
		do_dump = 1;

		if(do_dump && !ft->dump_stdout) {
			AM_IOERR_GOTO(ctx,
				      out_free,
				      AM_IOERR_FIND_RELATED,
				      "Frame type %s does not have a dump "
				      "function", ft->name);
		}

		if(!ft->read) {
			AM_IOERR_GOTO(ctx, out_free,
				      AM_IOERR_FIND_RELATED,
				      "Frame type %s does not have a read "
				      "function", ft->name);
		}

		if(ft->size > highest_size) {
			if(!(tmp = realloc(frame, ft->size))) {
				AM_IOERR_GOTO(ctx,
					      out_free,
					      AM_IOERR_ALLOC,
					      "Could not allocate space for "
					      "frame type %s.",
					      ft->name);
			}

			frame = tmp;
			highest_size = ft->size;
		}

		if(ft->read(ctx, frame)) {
			AM_IOERR_GOTO(ctx,
				      out_free,
				      AM_IOERR_READ_FRAME,
				      "Could not read frame of type "
				      "%s.",
				      ft->name);
		}

		if(check_offsets) {
			frame_end_offs = ftello(ctx->fp);

			/* Do not dump frames which occure before the start
			 * offset */
			if(frame_end_offs < start_offs)
				do_dump = 0;
		}

		if(do_dump) {
			if(!is_first)
				puts("\n");

			is_first = 0;

			if(dump_offsets)
				printf("# Offset: %jd\n", frame_start_offs);

			if(ft->dump_stdout(ctx, frame, 0, 0)) {
				AM_IOERR_GOTO(ctx,
					      out_destroy,
					      AM_IOERR_UNKNOWN,
					      "Could not dump frame of type "
					      "%s.",
					      ft->name);
			}
		}

		/* Process frame type associations */
		if(type_id == 0) {
			if(am_dsk_frame_type_id_process(ctx, frame)) {
				AM_IOERR_GOTO_NA(ctx,
						 out_destroy,
						 AM_IOERR_POSTPROCESS,
						 "Could not process frame type "
						 "ID association.");
			}
		}

		if(ft->destroy)
			ft->destroy(frame);
	}

	ret = 0;

out_destroy:
	if(frame && ft->destroy)
		ft->destroy(frame);
out_free:
	free(frame);

	return ret;
}

int am_dsk_dump_trace(struct am_io_context* ctx,
		      const char* filename,
		      off_t start_offs,
		      off_t end_offs,
		      int dump_offsets)
{
	struct am_dsk_header hdr;
	int ret = 1;
	int header_dumped = 0;

	if(end_offs != 0 && end_offs < start_offs) {
		AM_IOERR_GOTO_NA(ctx,
				 out,
				 AM_IOERR_CONVERT,
				 "End offset must not be smaller than start "
				 "offset");
	}

	if(am_io_context_open(ctx, filename, AM_IO_READ))
		goto out;

	if(am_dsk_header_read(ctx, &hdr))
		goto out_ctx;

	if(am_dsk_header_assert(ctx, &hdr))
		goto out_ctx;

	/* Check if header needs to be dumped */
	if(start_offs < ftello(ctx->fp)) {
		if(dump_offsets)
			puts("# Offset: 0");

		if(am_dsk_header_dump_stdout(ctx, &hdr, 0, 0))
			goto out_ctx;

		header_dumped = 1;
	}

	if(end_offs == 0 || ftello(ctx->fp) <= end_offs) {
		if(header_dumped)
			puts("\n");

		if(am_dsk_dump_frames(ctx, start_offs, end_offs, dump_offsets))
			goto out_ctx;
	}

	puts("");

	ret = 0;

out_ctx:
	am_io_context_close(ctx);
out:
	return ret;
}

/* Writes a frame type id frame to an output file using the output context ctx,
 * associating the type of the specified name with the given id.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int am_dsk_write_default_id(struct am_io_context* ctx,
					  char* name,
					  uint32_t id)
{
	struct am_dsk_frame_type_id fti;

	fti.id = id;
	fti.type_name.str = name;
	fti.type_name.len = strlen(name);

	return am_dsk_frame_type_id_write(ctx, 0, &fti);
}

{% for t in dsk_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.TemplatedGenerateFunctionTag) -%}
{% if not isinstance(tag, aftermath.tags.dsk.WriteToBufferFunction) and
      not isinstance(tag, aftermath.tags.dsk.WriteToBufferWithDefaultIDFunction) and
      not isinstance(tag, aftermath.tags.dsk.WriteDefaultIDToBufferFunction) %}
{{ tag.instantiateTemplate() }}

{% endif -%}
{% endfor -%}
{% endfor -%}
