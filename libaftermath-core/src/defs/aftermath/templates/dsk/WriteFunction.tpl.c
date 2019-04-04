/* Writes a {{dsk_type.getEntity()}} to disk.
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	{%- if gen_tag.hasTypeParam() %}
	if(am_dsk_uint32_t_write(ctx, &type_id)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_WRITE_FIELD,
				 "Could not write frame type ID for "
				 "type \"{{dsk_type.getName()}}\".");
	}
	{%- endif %}

	{%- for field in dsk_type.getFields() %}
	{%- set field_tag = field.getType().getTagInheriting(aftermath.tags.dsk.WriteFunction) %}

	if({{field_tag.getFunctionName()}}(ctx, &dsk->{{field.getName()}})) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_WRITE_FIELD,
				 "Could not write field \"{{field.getName()}}\" "
				 "of type \"{{dsk_type.getName()}}\".");
	}
{# #}
	{%- endfor -%}
{# #}
	return 0;
}
