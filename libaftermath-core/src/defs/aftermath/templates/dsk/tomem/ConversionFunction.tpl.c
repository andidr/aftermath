{# Dirty workaround to avoid scoping problem with can_fail #}
{% set can_fail = {"value" : False} -%}
{# #}
/* Converts a {{dsk_type.getEntity()}} into its in-memory representation.
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	{%- if tag.getMemType().hasDefaultConstructor() %}
	{%- if can_fail.update({"value": True}) %}{% endif %}
	if({{tag.getMemType().getDefaultConstructorName()}}(mem)) {
		AM_IOERR_GOTO_NA(ctx, out_err_constructor,
				 AM_IOERR_INIT,
				 "Default constructor for a "
				 "{{mem_type.getEntity()}} failed.");
	}
	{%- endif %}
	{%- for (dsk_field, mem_field) in tag.getFieldMap() -%}
	{%- set field_tag = dsk_field.getType().getTagInheriting(aftermath.tags.dsk.tomem.ConversionFunction) %}

	{%- if dsk_field.isArray() %}
	{%- if can_fail.update({"value": True}) %}{% endif %}
	mem->{{mem_field.getName()}} = NULL;

	if(dsk->{{dsk_field.getArrayNumElementsFieldName()}} > 0) {
		if(!(mem->{{mem_field.getName()}} = am_alloc_array_safe(dsk->{{dsk_field.getArrayNumElementsFieldName()}}, sizeof(*mem->{{mem_field.getName()}})))) {
			AM_IOERR_GOTO_NA(ctx, out_err_{{dsk_field.getName()}}, AM_IOERR_ALLOC,
					 "Could not allocate elements for array '{{mem_field.getName()}}' "
					 "of {{mem_type.getEntity()}}.");
		}
	}

	for(size_t i = 0; i < dsk->{{dsk_field.getArrayNumElementsFieldName()}}; i++) {
		{% if dsk_field.getType().isCompound() -%}
		if({{field_tag.getFunctionName()}}(ctx, &dsk->{{dsk_field.getName()}}[i], &mem->{{mem_field.getName()}}[i])) {
			{% if mem_field.getType().hasDestructor() -%}
			for(size_t j = 0; j < i; j++)
				{{mem_field.getType().getDestructorName()}}(&mem->{{mem_field.getName()}}[j]);
			{% endif -%}

			free(mem->{{mem_field.getName()}});

			AM_IOERR_GOTO(ctx, out_err_{{dsk_field.getName()}}, AM_IOERR_ALLOC,
				      "Could not assign element with index %zu of field '{{dsk_field.getName()}}' "
				      "to target array in in-memory representation.", i);
		}
		{% else -%}
		mem->{{mem_field.getName()}}[i] = dsk->{{dsk_field.getName()}}[i];
		{%- endif %}
	}
	{%- elif dsk_field.getType().isCompound() and dsk_field.getType().hasDestructor() %}
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
	{% if mem_field.isArray() -%}
	{% if mem_field.getType().hasDestructor() %}
	for(size_t i = 0; i < {{dsk_field.getArrayNumElementsFieldName()}}; i++)
		{{mem_field.getType().getDestructorName()}}(&mem->{{mem_field.getName()}}[i]);
	{% endif %}
	free(mem->{{mem_field.getName()}});
	{% elif mem_field.getType().hasDestructor() -%}
	{% if not can_fail -%}
	{{mem_field.getType().getDestructorName()}}(&mem->{{mem_field.getName()}});
	{%- endif %}
	{%- if can_fail.update({"value": True}) %}{% endif %}
	{%- endif -%}
{# #}
	{%- if mem_field.isArray() or mem_field.getType().hasDestructor() %}
out_err_{{dsk_field.getName()}}:
	{%- endif -%}
	{%- endfor %}
	{%- if can_fail["value"] %}
{%- if tag.getMemType().hasDefaultConstructor() %}
out_err_constructor:
{%- endif %}
	return 1;
	{%- endif %}
}
