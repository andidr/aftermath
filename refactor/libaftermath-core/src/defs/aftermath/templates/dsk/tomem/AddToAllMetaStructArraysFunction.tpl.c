{%- for dtms in dsktometa_tag.getSources() %}
{%- set meta_type = dtms.getMetaType() %}

{%- if isinstance(dtms, aftermath.tags.dsk.tomem.join.JoinTarget) %}
	{%- set append_per_trace_tag = meta_type.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendAndSetIndexFunction) %}
	{%- set append_per_ecoll_tag = meta_type.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendAndSetIndexFunction) %}
	{%- set append_per_ecoll_sub_tag = meta_type.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendAndSetIndexFunction) %}
{%- elif isinstance(dtms, aftermath.tags.dsk.tomem.join.JoinSource) %}
	{%- set append_per_trace_tag = meta_type.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction) %}
	{%- set append_per_ecoll_tag = meta_type.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction) %}
	{%- set append_per_ecoll_sub_tag = meta_type.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction) %}
{%- endif %}

{%- if not (append_per_trace_tag or append_per_ecoll_tag or append_per_ecoll_sub_tag) %}
	{{"Type does not have a supported AppendAndSetIndexFunction"/0}}
{%- endif %}
/* Adds an entry to the appropriate meta structure array for the meta type
 * {{dtms.getMetaType().getName()}}, providing the value of the field
 * '{{dtms.getDskField().getName()}}' of the corresponding on-disk structure
 * '{{dtms.getDskType().getName()}}'.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int {{meta_type.getName() }}_add(
	struct am_io_context* ctx,
	{{dtms.getDskType().getCType() }}* dsk,
	{{dtms.getMemType().getCType() }}* mem)
{
	struct {{meta_type.getName()}} meta;

	meta.id = dsk->{{dtms.getDskField().getName()}};
{# #}

	{%- if append_per_trace_tag %}
	if({{append_per_trace_tag.getFunctionName()}}(ctx, &meta))
		return 1;
	{%- endif %}

	{%- if append_per_ecoll_tag %}
	if({{append_per_ecoll_tag.getFunctionName()}}(ctx, dsk->{{dtms.getDskType().getTagInheriting(aftermath.tags.dsk.tomem.GeneratePerEventCollectionArrayFunction).getEventCollectionDskIDField().getName()}}, &meta))
		return 1;
	{%- endif %}

	{%- if append_per_ecoll_sub_tag %}
	if({{append_per_ecoll_sub_tag.getFunctionName()}}(ctx,
			dsk->{{dtms.getDskType().getTagInheriting(aftermath.tags.dsk.tomem.GeneratePerEventCollectionSubArrayFunction).getEventCollectionDskIDField().getName()}},
			dsk->{{dtms.getDskType().getTagInheriting(aftermath.tags.dsk.tomem.GeneratePerEventCollectionSubArrayFunction).getEventIDDskField().getName()}},
			&meta))
		return 1;
	{%- endif %}

	return 0;
}
{%- endfor %}
{# #}
/* Adds entries to all meta structure arrays used to match the fields of
 * {{gen_tag.getType().getName()}} pointing to other data structures or where
 * {{gen_tag.getType().getName()}} is the target structure referenced by another
 * source.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{%- for dtms in dsktometa_tag.getSources() %}
	if({{dtms.getMetaType().getName()}}_add(ctx, dsk, mem))
		return 1;
{# #}
	{%- endfor %}
	return 0;
}
