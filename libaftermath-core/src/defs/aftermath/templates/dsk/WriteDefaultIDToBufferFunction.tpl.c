/* Writes a frame type id frame to the write buffer wb, associating the type
 * {{gen_tag.getType().getName()}} with its default id.
 *
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	return am_dsk_write_default_id_to_buffer(
		wb,
		"{{gen_tag.getType().getName()}}",
		am_default_on_disk_type_ids.{{gen_tag.getType().getName()}});
}
