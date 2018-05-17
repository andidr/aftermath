{% include "fnproto.tpl.h" %}
{
	uintptr_t tmp;

	{% for m in pargs.field_mapping -%}
	assert(sizeof(tmp) >= sizeof(dsk->{{m[0]}}));
	{%- endfor %}

	{% for m in pargs.field_mapping -%}
	tmp = dsk->{{m[0]}};
	out->{{m[1]}} = (void*)tmp;
	{%- endfor %}

	return 0;
}
