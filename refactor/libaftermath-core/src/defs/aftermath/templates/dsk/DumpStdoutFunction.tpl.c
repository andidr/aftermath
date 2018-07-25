/* Dumps the contents of a {{dsk_type.getName()}} to stdout.  The parameter
 * indent indicates by how many tabs the first line should be indented, while
 * next_indent indicates by how many tabs the following lines belonging to this
 * structure should be indicated.
 *
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	am_fprintf_prefix(stdout, "\t", indent, "{{dsk_type.getName()}} {\n");

	{% for field in dsk_type.getFields() -%}
	{% set fieldtag = field.getType().getTagInheriting(aftermath.tags.dsk.DumpStdoutFunction) -%}
	am_fprintf_prefix(stdout, "\t", next_indent+1, "{{field.getName()}}: ");

	if({{ fieldtag.getFunctionName() }}(ctx, &dsk->{{field.getName()}}, indent, next_indent+1))
		return 1;

	{% if not loop.last -%}
	printf(",\n");
	{% else %}
	printf("\n");
	{% endif -%}
	{% endfor -%}
	am_fprintf_prefix(stdout, "\t", next_indent, "}");

	return 0;
}
