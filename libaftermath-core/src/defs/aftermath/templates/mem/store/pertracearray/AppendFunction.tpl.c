/* Appends a new element pointed to by 'e' at the end of the per-trace array for
 * a {{mem_type.getEntity()}}. If the array does not exist, a new array is
 * created.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	struct {{trace_array_struct_name}}* arr;

	if(!(arr = am_trace_find_or_add_trace_array(ctx->trace,
						    "{{trace_array_ident}}")))
	{
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ALLOC,
				 "Could not allocate trace array for a "
				 "{{mem_type.getEntity()}}.");
	}

	if({{trace_array_struct_name}}_appendp(arr, e)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ALLOC,
				 "Could not allocate space for a "
				 "{{mem_type.getEntity()}}.");
	}

	return 0;
}
