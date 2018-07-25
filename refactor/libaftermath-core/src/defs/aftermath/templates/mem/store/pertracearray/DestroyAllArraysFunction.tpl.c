{% set mem_type = gen_tag.getType() %}

/* Destroys all arrays of {{mem_type.getIdent()}}.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	am_array_collection_destroy_array(&ctx->trace->trace_arrays,
					  &ctx->trace->array_registry,
					  "{{mem_type.getIdent()}}");

	return 0;
}
