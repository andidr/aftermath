static inline void {{t.name}}_destroy({{t.c_type}}* f)
{
	{%- for field in t.fields -%}
	{%- set field_type = dsk.find(field.type) -%}
	{%- if field_type.destructor %}
	{%- if field_type.is_pointer %}
	{{field_type.destructor}}(f->{{field.name}});
	{%- else %}
	{{field_type.destructor}}(&f->{{field.name}});
	{%- endif -%}
	{%- endif -%}
	{%- endfor -%}
{# #}
}
