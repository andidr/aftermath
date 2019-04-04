/* Converts the in-memory representation of an on-disk {{dsk_type.getEntity()}}
 * into its on-disk representation and writes the result into the write buffer
 * passed as an argument.
 *
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	size_t old_used = wb->used;
{%- if gen_tag.hasConstantTypeID() %}
	uint32_t type_id = {{ gen_tag.getConstantTypeID() }};
{%- endif %}
{# #}
{%- if gen_tag.hasTypeParam() or gen_tag.hasConstantTypeID() %}
	if(am_dsk_uint32_t_write_to_buffer(wb, &type_id))
		goto out_err;
{# #}
{%- endif %}
	{%- for field in dsk_type.getFields() %}
	{%- set field_tag = field.getType().getTagInheriting(aftermath.tags.dsk.WriteToBufferFunction) %}
	if({{field_tag.getFunctionName()}}(wb, &e->{{field.getName()}}))
		goto out_err;
{# #}
	{%- endfor -%}
{# #}
	return 0;

out_err:
	wb->used = old_used;
	return 1;
}
