/* Appends a new element pointed to by 'e' at the end of the
 * per-event-collection sub-array for a {{mem_type.getEntity()}} for the event
 * collection identified by 'ecoll_id' and the sub-array identified by
 * 'sub_id'. If the array and / or sub-array do(es) not exist, new array(s)
 * is/are created.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	struct am_trace* t = ctx->trace;
	struct {{ecoll_array_struct_name}}* ecoll_arr;
	struct {{event_array_struct_name}}* sub_arr;
	struct am_event_collection* ecoll;
	size_t idx;

	if(!(ecoll = am_event_collection_array_find(&t->event_collections,
						    ecoll_id)))
	{
		AM_IOERR_RET1(ctx, AM_IOERR_FIND_RELATED,
			      "Could not find event collection with id "
			      "%" AM_EVENT_COLLECTION_ID_T_FMT ".",
			      ecoll_id);
	}

	if(!(ecoll_arr = am_event_collection_find_or_add_event_array(
		     &ctx->trace->array_registry,
		     ecoll,
		     "{{event_array_ident}}")))
	{
		AM_IOERR_RET1_NA(ctx, AM_IOERR_FIND_RELATED,
				 "Could not find / add event collection array "
				 "for type {{mem_type.getEntity()}}.");
	}

	if(!(sub_arr = {{ecoll_array_struct_name}}_find_or_add(ecoll_arr, sub_id))) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_FIND_RELATED,
				 "Could not find / add per-event-collection "
				 "sub-array for type "
				 "'{{mem_type.getEntity()}}.'");
	}

	if({{event_array_struct_name}}_appendp(sub_arr, e)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ALLOC,
				 "Could not allocate space for a "
				 "{{mem_type.getEntity()}}.");
	}

	idx = arr->num_elements - 1;
	arr->elements[idx].idx = idx;

	return 0;
}
