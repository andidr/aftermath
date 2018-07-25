{% set mem_type = gen_tag.getType() %}

/* Destroys all arrays of {{mem_type.getIdent()}}.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	struct am_event_collection* ecoll;

	am_trace_for_each_event_collection(ctx->trace, ecoll) {
		am_array_collection_destroy_array(&ecoll->event_arrays,
						  &ctx->trace->array_registry,
						  "{{mem_type.getIdent()}}");
	}

	return 0;
}
