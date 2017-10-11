{%- if t.is_frame %}
	{%- set skip_fields = 1 %}
	{%- set fun_name = t.name+"_read_skip_type" %}
	{%- include "dsk_read_fun.tpl.c" %}
{# #}
{%- elif "dsk_read" in t.defs %}
	{%- set skip_fields = 0 %}
	{%- set fun_name = t.name+"_read" %}
	{%- include "dsk_read_fun.tpl.c" %}
{# #}
{%- endif %}
