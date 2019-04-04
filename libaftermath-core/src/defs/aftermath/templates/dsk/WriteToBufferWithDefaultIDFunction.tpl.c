/* Converts the in-memory representation of an on-disk {{dsk_type.getEntity()}}
 * into its on-disk representation and writes the result into the write buffer
 * passed as an argument using the type's default on-disk frame type ID.
 *
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	{% if wtb_tag.hasTypeParam() -%}
	uint32_t type_id = am_default_on_disk_type_ids.{{gen_tag.getType().getName()}};
	return {{wtb_tag.getFunctionName()}}(wb, e, type_id);
	{%- else -%}
	return {{wtb_tag.getFunctionName()}}(wb, e);
	{%- endif %}
}
