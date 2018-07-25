/* Sort the meta structure array {{t.getName()}}_array */
{{template.getSignature()}}
{
	struct {{t.getName()}}_array* a;
{# #}

	{%- if t.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendAndSetIndexFunction) %}
	a = am_array_collection_find(&ctx->trace->trace_arrays,
		"{{t.getIdent()}}");

	if(a)
		{{t.getName()}}_array_qsort(a->elements, a->num_elements);
	{%- elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendAndSetIndexFunction) %}
	am_trace_for_each_event_collection_array(ctx->trace, "{{t.getIdent()}}", sa)
		{{t.getName()}}_array_qsort(a->elements, a->num_elements);

	{%- elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendAndSetIndexFunction) %}
	{%- set gpecsaf = t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendAndSetIndexFunction) %}
	am_trace_for_each_event_collection_subarray(ctx->trace, "{{t.getIdent()}}", struct {{gpecsaf.getEventCollectionArrayStructName()}}, sa)
		{{t.getName()}}_array_qsort(a->elements, a->num_elements);
	{%- else %}
	{{"Unsupported storage class"/0}}
	{%- endif %}
{# #}
	return 0;
}
