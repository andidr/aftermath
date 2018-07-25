/* Checks whether the minimum and maximum timestamps of a trace need to be
 * updated from a {{t.getEntity()}}. */
{{ template.getSignature() }}
{
	{%- for accessor in gen_tag.getAccessors() %}
	if(e->{{accessor}} < ctx->trace->bounds.start)
		ctx->trace->bounds.start = e->{{accessor}};

	if(e->{{accessor}} > ctx->trace->bounds.end)
		ctx->trace->bounds.end = e->{{accessor}};
{# #}
	{%- endfor %}
	return 0;
}
