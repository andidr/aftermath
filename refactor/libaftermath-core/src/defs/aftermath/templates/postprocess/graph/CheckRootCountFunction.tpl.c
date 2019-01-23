/* Checks if the per-trace array identified by
 * '{{gen_tag.getRootPointerArrayIdent()}}' containing the roots of the graph of {{gen_tag.getType().getEntity()}}s has
{%- if gen_tag.getMinCount() == gen_tag.getMinCount() %}
 * a count of exactly {{gen_tag.getMinCount()}}.
{%- else %}
 * at least {{gen_tag.getMinCount()}} entries and at most {{gen_tag.getMaxCount()}} entries.
{%- endif %}
 *
 * Returns 0 if the root count is right, otherwise 1.
 */
{{ template.getSignature() }}
{
	struct {{gen_tag.getRootPointerArrayStructName()}}* rootp_array;
	size_t min_root_count = {{gen_tag.getMinCount()}};
	size_t max_root_count = {{gen_tag.getMaxCount()}};

	{% if gen_tag.triggersOnlyIfAtLeatsOneNode() %}
	struct {{gen_tag.getNodeArrayStructName()}}* node_array;
	int at_least_one_node = 0;

	{% if t.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction) -%}
	if((node_array = am_trace_find_trace_array(ctx->trace, "{{t.getIdent()}}")))
		if(node_array->num_elements > 0)
			at_least_one_node = 1;
	{%- elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction) -%}
	am_trace_for_each_event_collection_array(ctx->trace, "{{t.getIdent()}}", node_array) {
		if(node_array->num_elements > 0) {
			at_least_one_node = 1;
			break;
		}
	}
	{% elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction) -%}
	am_trace_for_each_event_collection_subarray(ctx->trace, "{{t.getIdent()}}", struct {{t.getName()}}_array, node_array) {
		if(node_array->num_elements > 0) {
			at_least_one_node = 1;
			break;
		}
	}
	{% else %}
	{{ "Unknown storage class"/0 }}
	{% endif %}

	if(!at_least_one_node)
		return 0;
	{%endif %}

	rootp_array = am_trace_find_or_add_trace_array(ctx->trace, "{{gen_tag.getRootPointerArrayIdent()}}");

	if(rootp_array) {
		if(rootp_array->num_elements < min_root_count ||
		   rootp_array->num_elements > max_root_count)
		{
			if(min_root_count == max_root_count) {
				AM_IOERR_RET1(ctx, AM_IOERR_POSTPROCESS,
					      "Expected exactly %zu roots for "
					      "'{{gen_tag.getRootPointerArrayIdent()}}', "
					      "but has %zu roots.",
					      min_root_count,
					      rootp_array->num_elements);
			} else {
				AM_IOERR_RET1(ctx, AM_IOERR_POSTPROCESS,
					      "Expected at least %zu roots and "
					      "at most %zu root for"
					      "'{{gen_tag.getRootPointerArrayIdent()}}', "
					      "but has %zu roots.",
					      min_root_count,
					      max_root_count,
					      rootp_array->num_elements);
			}
		}
	} else {
		if(min_root_count > 0) {
			AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
					 "No roots found in "
					 "'{{gen_tag.getRootPointerArrayIdent()}}'.");
		}
	}

	return 0;
}
