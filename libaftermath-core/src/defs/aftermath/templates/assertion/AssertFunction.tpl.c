/* Cheks a {{t.getEntity()}} for validity.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{%- for step in gen_tag.getSteps() %}
	if(!({{step.getExpression(t, "e", True)}})) {
		AM_IOERR_RET1_NA(ctx,
				 AM_IOERR_ASSERT,
				 "{{step.getErrorMessage()}}");
	}
	{%- endfor %}
{# #}
	return 0;
}
