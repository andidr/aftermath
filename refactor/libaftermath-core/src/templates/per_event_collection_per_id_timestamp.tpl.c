{%- set pargs = t.process.args -%}
{%- set memtype = mem.find(pargs.mem_struct_name) -%}

static inline int
{{t.name}}_process(struct am_io_context* ctx,
			  {{t.c_type}}* f)
{
	struct am_event_collection* e;
	struct am_trace* t = ctx->trace;
	struct {{pargs.ecoll_array_struct_name}}* ecoll_arr;
	struct {{pargs.id_array_struct_name}}* id_arr;
	am_timestamp_t ts_prev;
	uint64_t ts;

	ts = f->{{pargs.dsk_struct_timestamp_field}};

	if(!(e = am_event_collection_array_find(&t->event_collections, f->{{pargs.dsk_struct_ecoll_id_field}}))) {
		AM_IOERR_GOTO(ctx, out_err, AM_IOERR_FIND_RELATED,
			       "Could not find event collection with id %" AM_EVENT_COLLECTION_ID_T_FMT ".",
			       f->{{pargs.dsk_struct_ecoll_id_field}});
	}

	if(!(ecoll_arr = am_event_collection_find_or_add_event_array(&ctx->trace->event_array_registry, e, {{pargs.ecoll_array_type_id}}))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_FIND_RELATED,
				  "Could not find / add event collection array for type {{t.entity}}.");
	}

	if(!(id_arr = {{pargs.ecoll_array_struct_name}}_find_or_add(ecoll_arr, f->{{pargs.dsk_struct_id_field}}))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_FIND_RELATED,
				  "Could not find event collection array for type {{t.entity}}.");
	}

	if(id_arr->num_elements > 0) {
		ts_prev = id_arr->elements[id_arr->num_elements-1].{{pargs.mem_struct_timestamp_field}};

		if(ts_prev >= ts) {
			if(ts_prev == ts) {
				AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ASSERT,
						 "A {{t.entity}} occurs at the same time as the preceding {{t.entity}}.");
			} else {
				AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ASSERT,
						 "A {{t.entity}} occurs before the preceding {{t.entity}}.");
			}
		}
	}

	if({{pargs.id_array_struct_name}}_reserve_end(id_arr)) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ADD,
				 "Could not allocate space for a {{t.entity}}.");
	}

	if({{pargs.dsk_to_mem_function}}(ctx, f, &id_arr->elements[id_arr->num_elements])) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ADD,
				 "Could not add {{t.entity}}.");
	}

	id_arr->num_elements++;
	id_arr->num_free--;

	return 0;

out_err:
	return 1;
}
