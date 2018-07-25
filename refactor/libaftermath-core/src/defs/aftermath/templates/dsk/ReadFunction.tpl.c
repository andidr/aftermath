/* Reads a {{dsk_type.getEntity()}} from disk.
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	{%- set curr_out = "out_err" -%}
	{%- for field in dsk_type.getFields() %}
	{%- set field_tag = field.getType().getTagInheriting(aftermath.tags.dsk.ReadFunction) %}

	if({{field_tag.getFunctionName()}}(ctx, &dsk->{{field.getName()}})) {
		AM_IOERR_GOTO_NA(ctx, {{curr_out}}, AM_IOERR_READ_FIELD,
				 "Could not read field \"{{field.getName()}}\" "
				 "of type \"{{dsk_type.getName()}}\".");
	}
{# #}
	{%- if field.getType().hasDestructor() -%}
	{% set curr_out = "out_err_"+field.getName() -%}
	{%- endif -%}
	{%- endfor -%}
{# #}
	{% set assert_tag = dsk_type.getTagInheriting(aftermath.tags.assertion.AssertFunction) %}
	{% if assert_tag %}
	if({{assert_tag.getFunctionName()}}(ctx, dsk)) {
		AM_IOERR_GOTO_NA(ctx, {{curr_out}}, AM_IOERR_ASSERT,
				 "Assertion of type \"{{dsk_type.getName()}}\" "
				 "failed.");
	}
	{% endif %}
{# #}
	return 0;
{# #}
	{%- set is_first = True -%}
	{% for field in dsk_type.getFields()|reverse -%}
	{% if field.getType().hasDestructor() -%}
	{% if not is_first -%}
{# #}
out_err_{{field.getName()}}:
	{{field.getType().getDestructorName()}}(&dsk->{{field.getName()}});
	{%- endif -%}
	{%- endif -%}
	{% set is_first = False -%}
	{%- endfor -%}
{# #}
out_err:
	return 1;
}
