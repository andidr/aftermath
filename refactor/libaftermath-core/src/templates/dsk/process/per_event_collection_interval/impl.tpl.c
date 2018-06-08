{%- set pargs = process.args -%}
{%- set memtype = mem.find(pargs.mem_struct_name) -%}

{% include "fnproto.tpl.h" %}
{
	struct am_event_collection* e;
	struct am_trace* t = ctx->trace;
	struct {{pargs.ecoll_array_struct_name}}* a;
	struct am_interval* i_prev;
	struct am_dsk_interval* i;

	i = &f->{{pargs.dsk_struct_interval_field}};

	if(i->start > i->end) {
		AM_IOERR_GOTO(ctx, out_err, AM_IOERR_ASSERT,
			      "Interval of a {{t.entity}} ends before it starts: "
			      "start: %" AM_TIMESTAMP_T_FMT ", end: "
			      "%" AM_TIMESTAMP_T_FMT ".",
			      i->start, i->end);
	}

	if(!(e = am_event_collection_array_find(&t->event_collections, f->{{pargs.dsk_struct_ecoll_id_field}}))) {
		AM_IOERR_GOTO(ctx, out_err, AM_IOERR_FIND_RELATED,
			       "Could not find event collection with id %" AM_EVENT_COLLECTION_ID_T_FMT ".",
			       f->{{pargs.dsk_struct_ecoll_id_field}});
	}

	if(!(a = am_event_collection_find_or_add_event_array(&ctx->trace->event_array_registry, e, "{{pargs.ecoll_array_type_name}}"))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_FIND_RELATED,
				  "Could not find / add event collection array for type {{t.entity}}.");
	}

	if(a->num_elements > 0) {
		i_prev = &a->elements[a->num_elements-1].{{pargs.mem_struct_interval_field}};

		if(i_prev->end > i->start) {
			AM_IOERR_GOTO(ctx, out_err, AM_IOERR_ASSERT,
				      "Interval of a {{t.entity}} starts before the interval of the preceding {{t.entity}} end: "
				      "previous interval: "
				      "[%" AM_TIMESTAMP_T_FMT ", "
				      "%" AM_TIMESTAMP_T_FMT "], "
				      "current interval: "
				      "[%" AM_TIMESTAMP_T_FMT ", "
				      "%" AM_TIMESTAMP_T_FMT "].",
				      i_prev->start,
				      i_prev->end,
				      i->start,
				      i->end);
		}
	}

	if({{pargs.ecoll_array_struct_name}}_prealloc(a)) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ADD,
				 "Could not allocate space for a {{memtype.entity}}.");
	}

	if({{pargs.dsk_to_mem_function}}(ctx, f, &a->elements[a->num_elements])) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ADD,
				 "Could not add {{memtype.entity}}.");
	}

	a->num_elements++;
	a->num_free--;

	{% if pargs.index_to_id_mappings -%}
	{% for itim in pargs.index_to_id_mappings -%}
	struct am_index_to_id_map_u{{itim.id_bits}}* id_arr_{{itim.id_field_name}};

	if(!(id_arr_{{itim.id_field_name}} =
	     am_io_context_find_or_add_event_collection_associated_array(
		     ctx, e, "{{itim.name}}")))
	{
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ALLOC,
				 "Could not allocate index to ID mapping {{itim.name}} for {{memtype.entity}}.");
	}

	struct am_index_to_id_map_u{{itim.id_bits}}_entry entry_{{itim.id_field_name}} = {
		.index = id_arr_{{itim.id_field_name}}->num_elements,
		.id = f->{{itim.id_field_name}}
	};

	if(am_index_to_id_map_u{{itim.id_bits}}_appendp(id_arr_{{itim.id_field_name}},
							&entry_{{itim.id_field_name}}))
	{
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ALLOC,
				 "Could not allocate space for index to ID mapping {{itim.name}} for {{memtype.entity}}.");
	}
	{% endfor -%}
	{%- endif -%}

	return 0;

out_err:
	return 1;
}
