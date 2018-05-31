{%- set pargs = postprocess.args -%}
{%- set target_type = mem.find(pargs.target.type_name) %}

{% include "fnproto.tpl.h" %}
{
	struct {{pargs.source.trace_array_type}}* source_element_arr;
	struct {{pargs.target.trace_array_type}}* target_element_arr;
	typeof(source_element_arr->elements) source_element;
	typeof(target_element_arr->elements) target_element;
	uint{{pargs.id_bits}}_t target_id;
	struct am_index_to_id_map_u{{pargs.id_bits}}_entry* target_id_ptr;

	source_element_arr = am_trace_find_trace_array(ctx->trace, "{{pargs.source.trace_array_type_name}}");
	target_element_arr = am_trace_find_trace_array(ctx->trace, "{{pargs.target.trace_array_type_name}}");

	if(!source_element_arr)
		return 0;

	if(!target_element_arr) {
		AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
				 "Could not find trace array for entity "
				 "'{{target_type.entity}}' (array type name "
				 "{{pargs.target.trace_array_type_name}}).");
	}

	for(size_t i = 0; i < source_element_arr->num_elements; i++) {
		source_element = &source_element_arr->elements[i];
		target_id = ctx->index_to_id_maps.{{pargs.source.map}}.elements[i].id;
		target_id_ptr = am_index_to_id_map_u{{pargs.id_bits}}_bsearch(
			&ctx->index_to_id_maps.{{pargs.target.map}}, target_id);

		if(!target_id_ptr) {
			AM_IOERR_RET1(ctx, AM_IOERR_POSTPROCESS,
				      "Could not find {{target_type.entity}} with ID %" PRIu{{pargs.id_bits}} " "
				      "for {{memtype.entity}}.", target_id);
		}

		target_element = &target_element_arr->elements[target_id_ptr->index];
		source_element->{{pargs.source.pointer_field_name}} = target_element;
	}

	return 0;
}
