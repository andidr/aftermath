{%- set pargs = postprocess.args -%}
{%- set source_type = mem.find(pargs.source.type_name) %}
{%- set target_type = mem.find(pargs.target.type_name) %}

{% include "fnproto.tpl.h" %}
{
	struct {{pargs.source.type_name}}_array* source_element_arr;
	struct {{pargs.target.type_name}}_array* target_element_arr;
	typeof(source_element_arr->elements) source_element;
	typeof(target_element_arr->elements) target_element;
	uint{{pargs.id_bits}}_t target_id;
	struct am_index_to_id_map_u{{pargs.id_bits}}_entry* target_id_ptr;
	struct am_event_collection* coll;

	target_element_arr = am_trace_find_trace_array(ctx->trace, "{{pargs.target.trace_array_type_name}}");

	am_trace_for_each_event_collection(ctx->trace, coll) {
		if(!(source_element_arr = am_event_collection_find_event_array(coll, "{{pargs.source.ecoll_array}}")))
			continue;

		if(!target_element_arr) {
			AM_IOERR_RET1_NA(ctx, AM_IOERR_POSTPROCESS,
					 "Found event collection event array for {{source_type.entity}}, "
					 "but there is no trace array with {{target_type.entity}} to link to.");
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
	}

	return 0;
}
