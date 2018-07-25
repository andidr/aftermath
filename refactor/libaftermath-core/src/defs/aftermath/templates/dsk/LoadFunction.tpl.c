{%- if dsk_type.hasDestructor() %}
{%- set out_dest = "out_err_destroy" %}
{%- else  %}
{%- set out_dest = "out_err" %}
{%- endif %}

/* Loads a {{dsk_type.getEntity()}} from disk at the current position and
 * processes it. Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{{dsk_type.getCType()}} f;
	int ret = 1;

	if({{read_tag.getFunctionName()}}(ctx, &f))
		goto out_err;

	{%- set assert_tag = dsk_type.getTagInheriting(aftermath.tags.assertion.AssertFunction) %}
	{%- if assert_tag %}
{# #}
	if({{assert_tag.getFunctionName()}}(ctx, &f)) {
		AM_IOERR_GOTO_NA(ctx, {{out_dest}}, AM_IOERR_ASSERT,
				 "Assertion of {{dsk_type.getEntity()}} failed.");
		goto {{out_dest}};
	}
	{%- endif %}

	{%- set process_tag = dsk_type.getTagInheriting(aftermath.tags.process.ProcessFunction) %}
	{%- if process_tag %}
{# #}
	if({{dsk_type.getName()}}_process(ctx, &f))
		goto {{out_dest}};
	{%- endif %}

	ret = 0;
{# #}
{%- if dsk_type.hasDestructor() %}
out_err_destroy:
	{{dsk_type.getDestructorName()}}(&f);
	{%- endif %}
out_err:
	return ret;
}
