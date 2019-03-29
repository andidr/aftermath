{%- set conv_suffix = ("u" if not tag.getNumElementsField().getType().isSigned() else "u") ~ tag.getNumElementsField().getType().getNumBits() %}
{%- set num_field = tag.getNumElementsField() %}
{%- set array_field = tag.getArrayField() %}

/* Reads a {{dsk_type.getEntity()}} from disk.
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	size_t element_size = sizeof(*dsk->{{array_field.getName()}});
	size_t num_elements;

	{%- set curr_out = "out_err" -%}
	{%- for field in tag.getVerbatimFields() %}
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

	if(am_safe_size_from_{{conv_suffix}}(&num_elements, dsk->{{num_field.getName()}})) {
		AM_IOERR_GOTO(ctx, {{curr_out}}, AM_IOERR_ASSERT,
			      "Value %" {{num_field.getType().getFormatStringSym()}} " of field \"{{num_field.getName()}}\" "
			      "is not a valid size.",
			      dsk->{{num_field.getName()}});
	}

	dsk->{{array_field.getName()}} = NULL;

	if(num_elements > 0) {
		if(!(dsk->{{array_field.getName()}} = am_alloc_array_safe(num_elements, element_size))) {
			AM_IOERR_GOTO(ctx, {{curr_out}}, AM_IOERR_ASSERT,
				      "Could not allocate %zu elements for array \"{{array_field.getName()}}\" "
				      "of type \"{{dsk_type.getName()}}\".",
				      num_elements);
		}
	}
	{%- set curr_out = "out_err_array" %}

	for(size_t i = 0; i < num_elements; i++) {
		{% set array_field_read_tag = array_field.getType().getTagInheriting(aftermath.tags.dsk.ReadFunction) -%}
		if({{array_field_read_tag.getFunctionName()}}(ctx, &dsk->{{array_field.getName()}}[i])) {
			{%- if array_field.getType().hasDestructor() %}
			for(size_t j = 0; j < i; j++)
				{{array_field.getType().getDestructorName()}}(&{{array_field.getName()}}[j]);

			{% endif -%}

			AM_IOERR_GOTO(ctx, {{curr_out}}, AM_IOERR_READ_FIELD,
				      "Could not array with index %zu for array field "
				      "\"{{array_field.getName()}}\" "
				      "of type \"{{dsk_type.getName()}}\".",
				      i);
		}
	}

	return 0;
out_err_array:
	free(dsk->{{array_field.getName()}});
{# #}
	{%- set is_first = True -%}
	{% for field in tag.getVerbatimFields()|reverse -%}
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
