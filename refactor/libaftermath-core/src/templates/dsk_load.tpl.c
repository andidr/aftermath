{%- if t.destructor %}
{%- set out_dest = "out_err_destroy" %}
{%- else  %}
{%- set out_dest = "out_err" %}
{%- endif %}
static inline int {{t.name}}_load(struct am_io_context* ctx)
{
	{{t.c_type}} f;
	int ret = 1;

	if({{t.name}}_read_skip_type(ctx, &f))
		goto out_err;

	{%- if t.assert %}
	if({{t.name}}_assert(ctx, &f)) {
		AM_IOERR_GOTO_NA(ctx, {{out_dest}}, AM_IOERR_ASSERT,
				 "Assertion of {{t.entity}} failed.");
		goto {{out_dest}};
	}
	{%- endif %}

	if({{t.name}}_process(ctx, &f))
		goto {{out_dest}};

	{% if t.timestamp_min_max_update -%}
	if({{t.name}}_timestamp_min_max_update(ctx, &f))
		goto {{out_dest}};
	{%- endif %}

	ret = 0;
{# #}
	{%- if t.destructor %}
out_err_destroy:
	{{t.name}}_destroy(&f);
	{%- endif %}
out_err:
	return ret;
}
