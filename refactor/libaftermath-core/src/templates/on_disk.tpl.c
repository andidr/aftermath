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

#include <aftermath/core/on_disk.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <aftermath/core/counter_description_array.h>
#include <aftermath/core/measurement_interval_array.h>
#include <aftermath/core/openstream_task_instance_array.h>
#include <aftermath/core/openstream_task_period_array.h>
#include <aftermath/core/openstream_task_type_array.h>
#include <aftermath/core/state_description_array.h>

#include <aftermath/core/state_event_array.h>
#include <aftermath/core/counter_event_array_collection.h>

static int am_dsk_read_frames(struct am_io_context* ctx);

{% set dsk = am_types.dsk -%}
{%- set mem = am_types.mem -%}

{% for t in am_types.filter_list_hasdefs(dsk.types_list, ["dsk_read"]) -%}
{% include "dsk_load.tpl.fnproto.h" %}
{%- endfor %}

{% for t in am_types.filter_list_hasdefs(dsk.types_list, ["dsk_read"]) -%}
{% include "dsk_read.tpl.fnproto.h" %}
{%- endfor %}

{% for t in am_types.filter_list_hasdefs(dsk.types_list, ["destructor"]) -%}
{% include "destructor.tpl.fnproto.h" %}
{% endfor -%}

{% for t in am_types.filter_list_hasattrs(dsk.types_list, ["process"]) -%}
{% include "dsk/process/fnproto.tpl.h" %};
{% endfor -%}

{%- for t in am_types.filter_list_hasdefs(dsk.types_list, ["dsk_to_mem_copy_function"]) -%}
{% include "dsk_to_mem_copy.tpl.fnproto.h" %}
{%- endfor %}

{% for t in am_types.filter_list_hasattrs(dsk.types_list, ["timestamp_min_max_update"]) -%}
{% include "timestamp_min_max_update.tpl.fnproto.h" %}
{% endfor %}

{%- for memtype in am_types.filter_list_hasattrs(mem.types_list, ["postprocess"]) %}
{%- for postprocess in memtype.postprocess %}
{% include "mem/postprocess/" + postprocess.type + "/fnproto.tpl.h" %};
static int {{memtype.name}}_postprocess(struct am_io_context* ctx);
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

static inline void* am_dsk_malloc(struct am_io_context* ctx,
				  size_t size)
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

{%- for t in am_types.filter_list_hasdefs(dsk.types_list, ["dsk_to_mem_copy_function"]) -%}
{% include "dsk_to_mem_copy.tpl.c" %}
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
	if(am_dsk_read_uint32_t_ctx(ctx, &s->len)) {
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

static inline int am_dsk_string_write(struct am_io_context* ctx,
				      const struct am_dsk_string* s)
{
	if(am_dsk_write_uint32_t_ctx(ctx, &s->len)) {
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

static inline int am_dsk_interval_assert(struct am_io_context* ctx,
					 const struct am_dsk_interval* interval)
{
	if(interval->start > interval->end) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ASSERT,
				 "Interval ends before starting.");
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

{% for t in dsk.types_list -%}
{%- if t.compound and t.destructor and "destructor" in t.defs -%}
{% include "destructor.tpl.c" %}
{# #}
{% endif -%}
{%- endfor -%}

{%- for t in am_types.filter_list_hasattrs(dsk.types_list, ["process"]) %}
{%- for process in t.process %}
{% include "dsk/process/"+process.type+"/impl.tpl.c" %}
{% endfor -%}

{% include "dsk/process/impl.tpl.c" %}
{% endfor -%}

{% for t in dsk.types_list %}
{% include "dsk_read.tpl.c" %}
{% endfor %}

{% for t in am_types.filter_list_hasdefs(dsk.types_list, ["dsk_write"]) -%}
{% include "dsk_write.tpl.c" %}
{% endfor %}

{% for t in dsk.types_list -%}
{% if t.is_frame %}
{% include "dsk_load.tpl.c" %}
{% endif %}
{% endfor %}

{% for t in am_types.filter_list_hasattrs(dsk.types_list, ["timestamp_min_max_update"]) -%}
{% include "timestamp_min_max_update.tpl.c" %}
{# #}
{% endfor %}

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
		if(am_dsk_read_uint32_t_ctx(ctx, &type_id)) {
			if(feof(ctx->fp))
				return 0;
			else
				return 1;
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

{% for memtype in am_types.filter_list_hasattrs(mem.types_list, ["postprocess"]) %}
{%- for postprocess in memtype.postprocess %}
{% include "mem/postprocess/" + postprocess.type + "/impl.tpl.c" %}
{% endfor %}

/* Performs postprocessing of all {{memtype.entity}}s. Returns 0 on success,
 * otherwise 1. */
static int {{memtype.name}}_postprocess(struct am_io_context* ctx)
{
	{%- for postprocess in memtype.postprocess %}
	if({% include "mem/postprocess/" + postprocess.type + "/fname.tpl.h" %}(ctx))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
{%- endfor %}

/* Sorts all index to ID maps by ID */
static inline void am_dsk_sort_index_to_id_maps(struct am_io_context* ctx)
{
	{%- for memtype in am_types.filter_list_hasattrs(mem.types_list, ["index_to_id_maps"]) %}
	{%- for itim in memtype["index_to_id_maps"] %}
	{%- if not memtype.is_pointer and itim.sort %}
	am_index_to_id_map_u{{itim.id_bits}}_sort_by_id(&ctx->index_to_id_maps.{{itim.name}});
	{%- endif %}
	{%- endfor -%}
	{% endfor %}
}

/* Performs postprocessing of a trace after loading it into memory. Returns 0 on
 * success, otherwise 1. */
int am_dsk_postprocess(struct am_io_context* ctx)
{
	am_dsk_sort_index_to_id_maps(ctx);
{# #}
	{%- for memtype in am_types.filter_list_hasattrs(mem.types_list, ["postprocess"]) %}
	if({{memtype.name}}_postprocess(ctx))
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

	{% for t in dsk.types_list -%}
	{% if t.is_frame and "dsk_read" in t.defs %}
	if(am_frame_type_registry_add(r,
				      "{{t.name}}",
				      {{t.name}}_load))
	{
		return 1;
	}
	{% endif %}
	{%- endfor -%}

	if(!(ft = am_frame_type_registry_find(r, "am_dsk_frame_type_id")))
		return 1;

	/* Frame type ID frame always has ID 0 */
	if(am_frame_type_registry_set_id(r, ft, 0))
		return 1;

	return 0;
}
