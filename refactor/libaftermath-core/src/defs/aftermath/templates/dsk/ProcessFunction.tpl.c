/* Applies a sequence of processing steps to a {{dsk_type.getEntity()}} after
 * assignment.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{%- for step in gen_tag.getSteps() %}
	if({{step.getFunctionTag().getFunctionName()}}(ctx, e))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
