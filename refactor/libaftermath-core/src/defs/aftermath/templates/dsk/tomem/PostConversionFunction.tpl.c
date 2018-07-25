{{template.getSignature()}}
{
	{%- for step in gen_tag.getSteps() %}
	if({{step.getFunctionTag().getFunctionName()}}(ctx, dsk, mem))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
