{%- for js in sources_tag.getSources() %}
{%- set jt = js.getAssociatedJoinTarget() %}
{%- set jsmt = js.getMetaType() %}
{%- set jtmt = jt.getMetaType() %}

static inline int {{jsmt.getName() }}_match_meta_arrays_inner(
	struct am_io_context* ctx,
	struct am_typed_array_generic* src_meta_arr,
	struct am_typed_array_generic* src_arr)
{
	{{jsmt.getCType()}}* src_meta;
	{{js.getMemType().getCType()}}* src;
	struct {{jtmt.getName()}}_array* tgt_meta_arr;
	struct {{jtmt.getName()}}* tgt_meta;
	struct am_typed_array_generic* tgt_arr;
	{{jt.getMemType().getCType()}}* tgt;

	if(src_meta_arr->num_elements != src_arr->num_elements) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
				 "Array with meta data does not have the same "
				 "number of elements as the array actual array "
				 "for type '{{js.getMemType().getName()}}'.");
	}

	{%- if jtmt.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendAndSetIndexFunction) %}
{# #}
	tgt_meta_arr = am_array_collection_find(&ctx->trace->trace_arrays,
					   "{{jtmt.getIdent()}}");
	tgt_arr = am_array_collection_find(&ctx->trace->trace_arrays,
					   "{{jt.getMemType().getIdent()}}");

	if(!tgt_arr || (tgt_arr && !tgt_meta_arr)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
				 "No array with meta data found for "
				 "type '{{jt.getMemType().getName()}}'.");
	}

	if(tgt_meta_arr->num_elements != tgt_arr->num_elements) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
				 "Array with meta data does not have the same "
				 "number of elements as the array actual array "
				 "for type '{{jt.getMemType().getName()}}'.");
	}
	{%- endif %}

	for(size_t i = 0; i < src_meta_arr->num_elements; i++) {
		src_meta = AM_PTR_ADD(src_meta_arr->elements, i*sizeof(*src_meta));
		src = AM_PTR_ADD(src_arr->elements, i*sizeof(*src));
		tgt_meta = NULL;
{# #}
		{%- if jtmt.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendAndSetIndexFunction) %}
		if(tgt_meta_arr)
			tgt_meta = {{jtmt.getName()}}_array_bsearch(tgt_meta_arr, src_meta->id);

		{%- elif jtmt.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendAndSetIndexFunction) %}
		am_trace_for_each_event_collection_array(ctx->trace, "{{jtmt.getIdent()}}", tgt_meta_arr)
			if((tgt_meta = {{jtmt.getName()}}_array_bsearch(tgt_meta_arr, src_meta->id)))
				break;

		{%- elif jtmt.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendAndSetIndexFunction) %}
		{%- set gpecsaf = jtmt.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendAndSetIndexFunction) %}
		am_trace_for_each_event_collection_subarray(ctx->trace, "{{jtmt.getIdent()}}", struct {{gpecsaf.getEventCollectionArrayStructName()}}, src_meta_arr)
			if((tgt_meta = {{jtmt.getName()}}_array_bsearch(tgt_meta_arr, src_meta->id)))
				break;
		{%- else %}
		{{"Unsupported storage class"/0}}
		{%- endif %}
{# #}
		{%- if not js.nullAllowed() %}
		if(!tgt_meta) {
			AM_IOERR_RET1(ctx, AM_IOERR_POSTPROCESS,
				      "No target structure found to set "
				      "field '{{js.getMemField().getName()}}' "
				      "of type '{{js.getMemType().getName()}}' "
				      "for ID %" {{jsmt.getFields().getFieldByName("id").getType().getFormatStringSym()}} ".",
				      src_meta->id);
		}
		{%- endif %}

		if(!tgt_meta) {
			src->{{js.getMemField().getName()}} = NULL;
		} else {
			tgt = AM_PTR_ADD(tgt_arr->elements, tgt_meta->idx * sizeof(*tgt));
			src->{{js.getMemField().getName()}} = tgt;
		}
	}

	return 0;
}

/* Matches the each instance of the source meta structure {{jsmt.getName()}}
 * with an instance of the target meta structure {{jtmt.getName()}}.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int {{jsmt.getName() }}_match_meta_arrays(struct am_io_context* ctx)
{
	struct am_typed_array_generic* src_meta_arr;
	struct am_typed_array_generic* src_arr;

	{%- if jsmt.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction) %}
{# #}
	src_meta_arr = am_array_collection_find(&ctx->trace->trace_arrays,
		"{{jsmt.getIdent()}}");
	src_arr = am_array_collection_find(&ctx->trace->trace_arrays,
		"{{js.getMemType().getIdent()}}");

	if((src_meta_arr && !src_arr) || (!src_meta_arr && src_arr))
		goto out_err;

	if(src_meta_arr)
		if({{jsmt.getName() }}_match_meta_arrays_inner(ctx, src_meta_arr, src_arr))
			goto out_err;

	{%- elif jsmt.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction) %}
	struct am_event_collection_array_iter iter;
{# #}
	am_trace_iter_each_event_collection_array(ctx->trace, "{{jsmt.getIdent()}}", iter) {
		src_meta_arr = iter.arr;
		src_arr = am_event_collection_find_event_array(
			iter.ecoll, "{{js.getMemType().getIdent()}}");

		if(src_meta_arr && !src_arr)
			goto out_err;

		if({{jsmt.getName() }}_match_meta_arrays_inner(ctx, src_meta_arr, src_arr))
			goto out_err;
	}

	{%- elif jsmt.getTagInheriting(aftermath.tags.mem.store.GeneratePerEventCollectionSubArrayFunction) %}
	{%- set gpecsaf = jsmt.getTagInheriting(aftermath.tags.mem.store.GeneratePerEventCollectionSubArrayFunction) %}
	am_trace_for_each_event_collection_subarray(ctx->trace, "{{jsmt.getIdent()}}", struct {{gpecsaf.getEventCollectionArrayStructName()}}, src_meta_arr)
		if({{jsmt.getName() }}_match_meta_arrays_inner(ctx, src_meta_arr))
			goto out_err;
	{%- else %}
	{{"Unsupported storage class"/0}}
	{%- endif %}

	return 0;

out_err:
	AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
			 "Could not match arrays for meta structures "
			 "'{{js.getMetaType().getName()}}' and "
			 "'{{jt.getMetaType().getName()}}' to connect "
			 "types '{{js.getMemType().getName()}}' and "
			 "'{{jt.getMemType().getName()}}'.");
}
{%- endfor %}

/* Match all source meta structures with target meta structures.
 *
 * Returns 0 on success, otherwise 1.
 */
{{template.getSignature()}}
{
	{%- for js in sources_tag.getSources() %}
	if({{js.getMetaType().getName()}}_match_meta_arrays(ctx))
		return 1;
	{%- endfor %}

	return 0;
}
