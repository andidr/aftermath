{% set dsktype = t -%}
{% set memtype = mem.find_same_dsk(dsktype.name) -%}

{% if t.process_dsk_mem %}
{% for process in t.process_dsk_mem %}
{% set pargs = process.args %}
{% include "./dsk/process_dsk_mem/" + process.type + "/impl.tpl.c" %}
{% endfor %}
{# #}
static inline int {{dsktype.name}}_process_dsk_mem(struct am_io_context* ctx,
						   {{dsktype.c_type}}* dsk,
						   {{memtype.c_type}}* out)
{
	{% for process in t.process_dsk_mem -%}
	if({% include "./dsk/process_dsk_mem/" + process.type + "/fname.tpl.h" %}(ctx, dsk, out))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
{% endif -%}

{# Dirty workaround to avoid scoping problem with can_fail #}
{% set can_fail = {"value" : False} -%}
{# #}
static inline int {{dsktype.name}}_to_mem(struct am_io_context* ctx,
					  {{dsktype.c_type}}* dsk,
					  {{memtype.c_type}}* out)
{
	{%- for copydef in dsktype.to_mem_copy_fields -%}
	{%- if copydef is string -%}
	   {%- set mem_field = am_types.find_field(memtype, copydef) -%}
	   {%- set dsk_field = am_types.find_field(dsktype, copydef) -%}
	   {%- set dsk_field_type = dsk.find(dsk_field.type) -%}
	{%- elif am_types.istuple(copydef) -%}
	   {%- set mem_field = am_types.find_field(memtype, copydef[1]) -%}
	   {%- set dsk_field = am_types.find_field(dsktype, copydef[0]) -%}
	   {%- set dsk_field_type = dsk.find(dsk_field.type) -%}
	{%- endif -%}
	{% if dsk_field_type.needs_constructor %}
	if({{dsk_field.type}}_to_mem(ctx, &dsk->{{dsk_field.name}}, &out->{{mem_field.name}})) {
		AM_IOERR_GOTO_NA(ctx, out_err_{{dsk_field.name}}, AM_IOERR_ALLOC,
				 "Could not assign field '{{dsk_field.name}}' of on-disk {{memtype.entity}} to in-memory representation.");
	}
{# #}
	{%- elif dsk_field_type.compound %}
	{{dsk_field.type}}_to_mem(ctx, &dsk->{{dsk_field.name}}, &out->{{mem_field.name}});
{# #}
	{%- else %}
	out->{{mem_field.name}} = dsk->{{dsk_field.name}};
{# #}
	{%- endif -%}
	{%- endfor -%}

	{% if t.process_dsk_mem %}
	{% if can_fail.update({"value": True}) %}{% endif -%}
	if({{dsktype.name}}_process_dsk_mem(ctx, dsk, out)) {
		AM_IOERR_GOTO_NA(ctx, out_err_dsk_mem_process, AM_IOERR_CONVERT,
				 "Could not carry out final processing of in-memory type '{{memtype.entity}}'");
	}
	{%- endif %}
{# #}
	return 0;
{# #}
{% if t.process_dsk_mem %}
out_err_dsk_mem_process:
{% endif %}
{%- for copydef in dsktype.to_mem_copy_fields|reverse() -%}
	{%- if copydef is string -%}
	  {% set dsk_field = am_types.find_field(dsktype, copydef) -%}
	{%- elif am_types.istuple(copydef) -%}
	  {% set dsk_field = am_types.find_field(dsktype, copydef[0]) -%}
	{%- endif -%}

	{% set dsk_field_type = dsk.find(dsk_field.type) -%}
	{% set mem_field_type = mem.find_same_dsk(dsk_field.type) -%}

	{% if dsk_field_type.needs_constructor -%}
	{% if can_fail.update({"value": True}) %}{% endif -%}
	{% if not loop.first %}
	{{mem_field_type.destructor}}(&out->{{dsk_field.name}});
	{%- endif %}
out_err_{{dsk_field.name}}:
	{%- endif -%}
	{%- endfor %}
	{%- if can_fail["value"] %}
	return 1;
	{%- endif %}
}
