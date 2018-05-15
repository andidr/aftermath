{% include "fnproto.tpl.h" %}
{
	{%- for process in t.process %}
	if({% include process.type+"/fname.tpl.h" %}(ctx, f))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
