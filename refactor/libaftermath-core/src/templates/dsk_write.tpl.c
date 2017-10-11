int {{t.name}}_write(struct am_io_context* ctx,
			    const {{t.c_type}}* f)
{
	{%- for field in t.fields -%}

	{%- if am_types.isinttype(field.type) -%}
		{%- set write_fun = "am_dsk_write_"+field.type -%}
	{%- else -%}
		{%- set write_fun = field.type+"_write" -%}
	{%- endif %}
	if({{write_fun}}(ctx, &f->{{field.name}})) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_WRITE_FIELD,
				 "Could not write field \"{{field.name}}\" "
				 "of type \"{{t.entity}}\".");
	}
{# #}
	{%- endfor %}
	return 0;
}
{# #}
