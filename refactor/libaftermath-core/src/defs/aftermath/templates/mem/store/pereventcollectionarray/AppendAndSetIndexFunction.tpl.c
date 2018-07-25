/* Appends a new element pointed to by 'e' at the end of the
 * per-event-collection array for a {{mem_type.getEntity()}} for the event
 * collection identified by 'ecoll_id'. If the array does not exist, a new array
 * is created.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	struct am_trace* t = ctx->trace;
	struct {{event_array_struct_name}}* arr;
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

	if(!(arr = am_event_collection_find_or_add_event_array(
		     &ctx->trace->array_registry,
		     ecoll,
		     "{{event_array_ident}}")))
	{
		AM_IOERR_RET1_NA(ctx, AM_IOERR_FIND_RELATED,
				 "Could not find / add event collection array "
				 "for type {{mem_type.getEntity()}}.");
	}

	if({{event_array_struct_name}}_appendp(arr, e)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ALLOC,
				 "Could not allocate space for a "
				 "{{mem_type.getEntity()}}.");
	}

	idx = arr->num_elements - 1;
	arr->elements[idx].idx = idx;

	return 0;
}
