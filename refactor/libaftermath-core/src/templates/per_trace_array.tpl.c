{%- set pargs = t.process.args -%}
{%- set memtype = mem.find(pargs.mem_struct_name) -%}

static inline int {{t.name}}_process(struct am_io_context* ctx,
				     {{t.c_type}}* f)
{
	struct {{pargs.trace_array_struct_name}}* a = &ctx->trace->{{pargs.trace_array_field}};
	{{memtype.c_type}}* mem;

	if(!(mem = {{pargs.trace_array_struct_name}}_reserve_sorted(a, f->{{pargs.dsk_struct_sort_field}}))) {
		AM_IOERR_GOTO_NA(ctx, out_err, AM_IOERR_ALLOC,
				 "Could not allocate space for a {{memtype.entity}}.");
	}

	if({{pargs.dsk_to_mem_function}}(ctx, f, mem)) {
		AM_IOERR_GOTO_NA(ctx, out_err_assign, AM_IOERR_CONVERT,
				 "Could not assign values from an on-disk {{memtype.entity}}.");
	}

	return 0;

out_err_assign:
	{{pargs.trace_array_struct_name}}_removep(a, mem);
out_err:
	return 1;
}