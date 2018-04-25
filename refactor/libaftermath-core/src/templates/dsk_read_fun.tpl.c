static inline int {{fun_name}}(struct am_io_context* ctx, {{t.c_type}}* f)
{
	{%- set curr_out = "out_err" -%}
	{%- for field in t.fields[skip_fields:] %}
	{%- if am_types.isinttype(field.type) %}
		{%- set read_fun = "am_dsk_read_"+field.type+"_ctx" %}
	{% else %}
		{%- set read_fun = field.type+"_read" %}
	{% endif -%}
	if({{read_fun}}(ctx, &f->{{field.name}})) {
		AM_IOERR_GOTO_NA(ctx, {{curr_out}}, AM_IOERR_READ_FIELD,
				 "Could not read field \"{{field.name}}\" "
				 "of type \"{{t.entity}}\".");
	}
{# #}
		{%- if dsk.find(field.type).destructor -%}
			{% set curr_out = "out_err_"+field.name -%}
		{%- endif -%}
	{%- endfor -%}
{# #}
	return 0;
{# #}
	{%- set is_first = True -%}
	{% for field in t.fields[skip_fields:]|reverse -%}
	{% if dsk.find(field.type).destructor -%}
	{% if not is_first -%}
{# #}
out_err_{{field.name}}:
	{{dsk.find(field.type).destructor}}(&f->{{field.name}});
	{%- endif -%}
	{%- endif -%}
	{% set is_first = False -%}
	{%- endfor -%}
{# #}
out_err:
	return 1;
}
