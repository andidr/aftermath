{%- set t = gen_tag.getType() %}
{%- set sources_tag = t.getTagInheriting(aftermath.tags.mem.collect.CollectSources) %}

{%- for source in sources_tag.getSources() %}
{%- set target = source.getAssociatedTarget() %}
/* Adds a single instance of '{{source.getType().getName()}}' pointing to
 * '{{target.getType().getName()}}' with the field
 * '{{source.getPtrField().getName()}}' a to the array
 * '{{source.getAssociatedTarget().getArrayField().getName()}} of
 * '{{target.getType().getName()}}'.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int {{template.getFunctionName()}}_{{source.getPtrField().getName()}}_{{target.getType().getName()}}_{{target.getArrayField().getName()}}_add(struct am_io_context* ctx, {{source.getType().getCType()}}* e)
{
	size_t new_num;
	void* tmp;

	{%- if source.getSelfMode() == aftermath.tags.mem.collect.SelfMode.IGNORE %}
	if((uintptr_t)e->{{source.getPtrField().getName()}} == (uintptr_t)e)
		return 0;
	{%- elif source.getSelfMode() == aftermath.tags.mem.collect.SelfMode.ERROR %}
	if((uintptr_t)e->{{source.getPtrField().getName()}} == (uintptr_t)e) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_FIND_RELATED,
				 "Field '{{source.getPtrField().getName()}}' "
				 "for an instance of type '{{source.getType().getName()}}' "
				 "points to the instance itself.");
	}
	{%- elif source.getSelfMode() == aftermath.tags.mem.collect.SelfMode.SETNULL %}
	if((uintptr_t)e->{{source.getPtrField().getName()}} == (uintptr_t)e)
		e->{{source.getPtrField().getName()}} = NULL;
	{%- endif -%}

	{%- if source.nullAllowed() %}
	if(!e->{{source.getPtrField().getName()}})
		return 0;
	{%- else %}
	if(!e->{{source.getPtrField().getName()}}) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_FIND_RELATED,
				 "NULL value for field '{{source.getPtrField().getName()}}' "
				 "for an instance of type '{{source.getType().getName()}}'");
	}
	{%- endif %}

	new_num = e->{{source.getPtrField().getName()}}->{{target.getNumField().getName()}};

	if(am_size_inc_safe(&new_num, 1)) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ALLOC,
				 "Could not increase array size of '{{target.getArrayField().getName()}}' "
				 "of an instance of'{{target.getType().getName()}}'.");
	}

	if(!(tmp = am_realloc_array_safe(e->{{source.getPtrField().getName()}}->{{target.getArrayField().getName()}},
					 new_num,
					 sizeof(*e->{{source.getPtrField().getName()}}->{{target.getArrayField().getName()}}))))
	{
		AM_IOERR_RET1_NA(ctx, AM_IOERR_ALLOC,
				 "Could not rezize array '{{target.getArrayField().getName()}}' "
				 "of an instance of '{{target.getType().getName()}}'.");
	}

	e->{{source.getPtrField().getName()}}->{{target.getArrayField().getName()}} = tmp;
	e->{{source.getPtrField().getName()}}->{{target.getArrayField().getName()}}[new_num-1] = e;
	e->{{source.getPtrField().getName()}}->{{target.getNumField().getName()}} = new_num;

	return 0;
}

/* Adds all instances of '{{source.getType().getName()}}' pointing to
 * '{{target.getType().getName()}}' with the field
 * '{{source.getPtrField().getName()}}' a to the array
 * '{{source.getAssociatedTarget().getArrayField().getName()}} of
 * '{{target.getType().getName()}}'.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int {{template.getFunctionName()}}_{{source.getPtrField().getName()}}_{{target.getType().getName()}}_{{target.getArrayField().getName()}}(struct am_io_context* ctx)
{
	struct {{t.getName()}}_array* array;

	{% if t.getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction) -%}
	if((array = am_trace_find_trace_array(ctx->trace, "{{t.getIdent()}}"))) {
		for(size_t i = 0; i < array->num_elements; i++) {
			if({{template.getFunctionName()}}_{{source.getPtrField().getName()}}_{{target.getType().getName()}}_{{target.getArrayField().getName()}}_add(ctx, &array->elements[i]))
				goto out_err;
		}
	}
	{%- elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction) -%}
	am_trace_for_each_event_collection_array(ctx->trace, "{{t.getIdent()}}", array) {
		for(size_t i = 0; i < array->num_elements; i++) {
			if({{template.getFunctionName()}}_{{source.getPtrField().getName()}}_{{target.getType().getName()}}_{{target.getArrayField().getName()}}_add(ctx, &array->elements[i]))
				goto out_err;
		}
	}
	{% elif t.getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction) -%}
	am_trace_for_each_event_collection_subarray(ctx->trace, "{{t.getIdent()}}", struct {{t.getName()}}_array, array) {
		for(size_t i = 0; i < array->num_elements; i++) {
			if({{template.getFunctionName()}}_{{source.getPtrField().getName()}}_{{target.getType().getName()}}_{{target.getArrayField().getName()}}_add(ctx, &array->elements[i]))
				goto out_err;
		}
	}
	{% else %}
	{{ "Unknown storage class"/0 }}
	{% endif %}

	return 0;

out_err:
	AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
			 "Could not collect all '{{source.getPtrField().getName()}}' pointers from type '{{source.getType().getName()}}' "
			 "in array '{{target.getArrayField().getName()}}' "
			 "of type '{{target.getType().getName()}}'.");
}
{% endfor %}

/* Adds the instances of all {{t.getEntity()}} instances to the
 * arrays of the instances of their configured collecting structures.
 *
 * Returns 0 on success, otherwise 1.
 */
{{ template.getSignature() }}
{
	{% for source in sources_tag.getSources() -%}
	{% set target = source.getAssociatedTarget() -%}
	if({{template.getFunctionName()}}_{{source.getPtrField().getName()}}_{{target.getType().getName()}}_{{target.getArrayField().getName()}}(ctx))
		return 1;
	{%- endfor %}
{# #}
	return 0;
}
