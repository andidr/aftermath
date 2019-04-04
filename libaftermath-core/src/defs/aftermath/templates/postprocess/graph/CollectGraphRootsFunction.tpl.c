{%- set t = gen_tag.getType() %}

/* Adds pointers to all structures of type '{{t.getName()}}' to the
 * per-trace array identified by '{{gen_tag.getTargetArrayIdent()}}' whose
 * field '{{gen_tag.getParentField().getName()}}' is NULL.
 *
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	struct {{t.getName()}}_array* src_array;
	struct {{gen_tag.getTargetArrayStructName()}}* tgt_array;
	{{t.getCType()}}* node;

	if(!(tgt_array = am_trace_find_or_add_trace_array(ctx->trace, "{{gen_tag.getTargetArrayIdent()}}"))) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
				 "Could not add per-trace array "
				 "'{{gen_tag.getTargetArrayIdent()}}'.");
	}

	{% if t.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction) -%}
	if((src_array = am_trace_find_trace_array(ctx->trace, "{{t.getIdent()}}"))) {
		for(size_t i = 0; i < src_array->num_elements; i++) {
			node = &src_array->elements[i];

			if(!node->{{gen_tag.getParentField().getName()}})
				if({{gen_tag.getTargetArrayStructName()}}_append(tgt_array, node))
					goto out_err;
		}
	}
	{%- elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction) -%}
	am_trace_for_each_event_collection_array(ctx->trace, "{{t.getIdent()}}", src_array) {
		for(size_t i = 0; i < src_array->num_elements; i++) {
			node = &src_array->elements[i];

			if(!node->{{gen_tag.getParentField().getName()}})
				if({{gen_tag.getTargetArrayStructName()}}_append(tgt_array, node))
					goto out_err;
		}
	}
	{% elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction) -%}
	am_trace_for_each_event_collection_subarray(ctx->trace, "{{t.getIdent()}}", struct {{t.getName()}}_array, src_array) {
		for(size_t i = 0; i < src_array->num_elements; i++) {
			node = &src_array->elements[i];

			if(!node->{{gen_tag.getParentField().getName()}})
				if({{gen_tag.getTargetArrayStructName()}}_append(tgt_array, node))
					goto out_err;
		}
	}
	{% else %}
	{{ "Unknown storage class"/0 }}
	{% endif %}

	return 0;

out_err:
	AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
			 "Could not collect graph roots for "
			 "'{{t.getName()}}'.");
}
