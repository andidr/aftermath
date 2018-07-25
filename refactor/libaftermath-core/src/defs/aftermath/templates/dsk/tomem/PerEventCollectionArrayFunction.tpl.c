/* Converts the on-disk representation of a {{dsk_type.getEntity()}} into its
 * in-memory representation and appends the result to the appropriate
 * per-event-collection array.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{{tomem_tag.getMemType().getCType()}} mem;

	if({{tomem_tag.getFunctionName()}}(ctx, dsk, &mem)) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_CONVERT,
				 "Could not assign values from an {{tomem_tag.getDskType().getEntity()}}.");
	}

	if({{store_tag.getFunctionName()}}(ctx, dsk->{{gen_tag.getEventCollectionDskIDField().getName()}}, &mem)) {
		AM_IOERR_GOTO_NA(ctx, out_err_destroy, AM_IOERR_ALLOC,
				 "Could not store {{tomem_tag.getMemType().getEntity()}} in per-event-collection array.");
	}

	{% set tomem_process = tomem_tag.getMemType().hasTag(aftermath.tags.process.ProcessFunction) %}
	{% if tomem_process %}
	if({{tomem_tag.getMemType().getTag(aftermath.tags.process.ProcessFunction).getFunctionName()}}(ctx, &mem)) {
		AM_IOERR_GOTO_NA(ctx, out_err_destroy, AM_IOERR_CONVERT,
				 "Could not process {{tomem_tag.getMemType().getEntity()}}.");
	}
	{% endif %}

{# #}
	return 0;

out_err_destroy:
	{% if tomem_process and tomem_tag.getMemType().hasDestructor() %}
	{{tomem_tag.getMemType().getDestructorName()}}(&mem);
	{% endif %}
out_err:
	return 1;
}
