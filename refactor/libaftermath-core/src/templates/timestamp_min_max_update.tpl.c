static inline int {{t.name}}_timestamp_min_max_update(struct am_io_context* ctx, {{t.c_type}}* f)
{
	{% if t.timestamp_min_max_update.type == "interval" -%}
	struct am_interval i;

	if(am_dsk_interval_to_mem(ctx, &f->{{t.timestamp_min_max_update.field}}, &i))
		return 1;

	if(!ctx->bounds_valid)
		ctx->trace->bounds = i;
	else
		am_interval_extend(&ctx->trace->bounds, &i);
	{%- elif t.timestamp_min_max_update.type == "discrete" -%}
	if(!ctx->bounds_valid) {
		ctx->trace->bounds.start = f->{{t.timestamp_min_max_update.field}};
		ctx->trace->bounds.end = f->{{t.timestamp_min_max_update.field}};
	} else {
		am_interval_extend_timestamp(&ctx->trace->bounds, f->{{t.timestamp_min_max_update.field}});
	}
	{%- endif %}

	ctx->bounds_valid = 1;

	return 0;
}
