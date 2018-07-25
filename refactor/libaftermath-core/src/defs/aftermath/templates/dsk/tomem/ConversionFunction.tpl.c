{# Dirty workaround to avoid scoping problem with can_fail #}
{% set can_fail = {"value" : False} -%}
{# #}
/* Converts a {{dsk_type.getEntity()}} into its in-memory representation.
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	{%- for (dsk_field, mem_field) in tag.getFieldMap() -%}
	{%- set field_tag = dsk_field.getType().getTagInheriting(aftermath.tags.dsk.tomem.ConversionFunction) %}

	{%- if dsk_field.getType().isCompound() and dsk_field.getType().hasDestructor() %}
	if({{field_tag.getFunctionName()}}(ctx, &dsk->{{dsk_field.getName()}}, &mem->{{mem_field.getName()}})) {
		AM_IOERR_GOTO_NA(ctx, out_err_{{dsk_field.getName()}}, AM_IOERR_ALLOC,
				 "Could not assign field '{{dsk_field.getName()}}' "
				 "of on-disk {{mem_type.getEntity()}} to "
				 "in-memory representation.");
	}
{# #}
	{%- elif dsk_field.getType().isCompound() %}
	{{field_tag.getFunctionName()}}(ctx, &dsk->{{dsk_field.getName()}}, &mem->{{mem_field.getName()}});
	{%- else %}
	mem->{{mem_field.getName()}} = dsk->{{dsk_field.getName()}};
	{%- endif -%}
	{%- endfor %}

	{% set assert_tag = tag.getMemType().getTagInheriting(aftermath.tags.assertion.AssertFunction) -%}
	{% if assert_tag -%}
	if({{assert_tag.getFunctionName()}}(ctx, mem)) {
		AM_IOERR_GOTO_NA(ctx, out_assert, AM_IOERR_ASSERT,
				 "Assertion of type \"{{tag.getMemType().getName()}}\" "
				 "failed.");
	}
	{% endif %}

	{% set postconv_tag = tag.getDskType().getTagInheriting(aftermath.tags.dsk.tomem.PostConversionFunction) -%}
	{% if postconv_tag -%}
	if({{postconv_tag.getFunctionName()}}(ctx, dsk, mem)) {
		AM_IOERR_GOTO_NA(ctx, out_postconv, AM_IOERR_CONVERT,
				 "Post conversion step of type \"{{tag.getMemType().getName()}}\" "
				 "failed.");
	}
	{% endif %}
{# #}
	return 0;

	{% if postconv_tag %}
	{%- if can_fail.update({"value": True}) %}{% endif %}
out_postconv:
	{% endif %}
	{% if assert_tag -%}
	{%- if can_fail.update({"value": True}) %}{% endif %}
out_assert:
	{% endif -%}
	{%- for (dsk_field, mem_field) in tag.getFieldMap()|reverse() -%}
	{% if dsk_field.getType().hasDestructor() -%}
	{% if not can_fail -%}
	{{mem_field.getType().getDestructorName()}}(&mem->{{dsk_field.getName()}});
	{%- endif %}
	{%- if can_fail.update({"value": True}) %}{% endif %}
{# #}
out_err_{{dsk_field.getName()}}:
	{%- endif -%}
	{%- endfor %}
	{%- if can_fail["value"] %}
	return 1;
	{%- endif %}
}
