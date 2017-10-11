{% set dsktype = t -%}
{% set memtype = mem.find_same_dsk(dsktype.name) -%}
{# Dirty workaround to avoid scoping problem with can_fail #}
{% set can_fail = {"value" : False} -%}
{# #}
static inline int {{dsktype.name}}_to_mem(struct am_io_context* ctx,
					  {{dsktype.c_type}}* dsk,
					  {{memtype.c_type}}* out)
{
	{%- for dsk_field_name in dsktype.to_mem_copy_fields -%}
	{%- set dsk_field = am_types.find_field(dsktype, dsk_field_name) -%}
	{%- set dsk_field_type = dsk.find(dsk_field.type) -%}
	{% if dsk_field_type.needs_constructor %}
	if({{dsk_field.type}}_to_mem(ctx, &dsk->{{dsk_field.name}}, &out->{{dsk_field.name}})) {
		AM_IOERR_GOTO_NA(ctx, out_err_{{dsk_field.name}}, AM_IOERR_ALLOC,
				 "Could not assign field '{{dsk_field.name}}' of on-disk {{memtype.entity}} to in-memory representation.");
	}
{# #}
	{%- elif dsk_field_type.compound %}
	{{dsk_field.type}}_to_mem(ctx, &dsk->{{dsk_field.name}}, &out->{{dsk_field.name}});
{# #}
	{%- else %}
	out->{{dsk_field.name}} = dsk->{{dsk_field.name}};
{# #}
	{%- endif -%}
	{%- endfor -%}
{# #}
	return 0;
{# #}
{%- for dsk_field_name in dsktype.to_mem_copy_fields|reverse() -%}
	{% set dsk_field = am_types.find_field(dsktype, dsk_field_name) -%}
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
