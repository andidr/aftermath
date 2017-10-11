static inline void {{t.name}}_destroy({{t.c_type}}* f)
{
	{% for field in t.fields -%}
	{%- if dsk.find(field.type).destructor -%}
	{{dsk.find(field.type).destructor}}(&f->{{field.name}});
	{%- endif -%}
	{%- endfor %}
}
