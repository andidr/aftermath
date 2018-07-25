/* Applies a sequence of teardown steps to a {{t.getEntity()}} after the trace
 * has been loaded, after postprocessing and after finalization.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{%- for step in gen_tag.getSteps() %}
	if({{step.getFunctionTag().getFunctionName()}}(ctx))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
