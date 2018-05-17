{%- set pargs = postprocess.args -%}
{%- set eltype = mem.find(pargs.element.type_name) %}
{%- set desctype = mem.find(pargs.description.type_name) %}

/* Changes the field {{pargs.element_field}} all {{eltype.entity}}s, such that
 * the value does not correspond to the field {{pargs.description_field}} of the
 * corresponding {{pargs.description_entity}}, but to the index of the
 * {{pargs.description.entity}} with that value in the
 * {{pargs.description.trace_array_field}} array of the trace.
 *
 * Returns 0 on success, otherwise 1.
 */
{% include "fnproto.tpl.h" %}
{
	struct am_event_collection* coll;
	struct am_trace* t = ctx->trace;
	struct {{pargs.element.type_name}}_array* el_arr;
	struct {{pargs.description.type_name}}_array* desc_arr;
	struct {{pargs.description.type_name}}* desc;
	struct {{memtype.name}}* e;
	size_t num_desc;

	desc_arr = &ctx->trace->{{pargs.description.trace_array_field}};
	num_desc = desc_arr->num_elements;

	/* If IDs are already sequential? */
	if(desc_arr->num_elements > 0 &&
	   desc_arr->elements[num_desc-1].{{pargs.description.field}} == num_desc-1)
	{
		/* Only check for the presence of a {{eltype.entity}} that does
		 * not have a description */
		am_trace_for_each_event_collection(t, coll) {
			if(!(el_arr = am_event_collection_find_event_array(coll, "{{pargs.element.ecoll_array_type_name}}")))
				continue;

			for(e = &el_arr->elements[0];
			    e != &el_arr->elements[el_arr->num_elements];
			    e++)
			{
				if(e->{{pargs.element.field}} >= num_desc)
					goto out_err;
			}
		}
	} else {
		/* Ids not already sequential, adjust every element */
		am_trace_for_each_event_collection(t, coll) {
			if(!(el_arr = am_event_collection_find_event_array(coll, "{{pargs.element.ecoll_array_type_name}}")))
				continue;

			for(e = &el_arr->elements[0];
			    e != &el_arr->elements[el_arr->num_elements];
			    e++)
			{
				desc = {{pargs.description.type_name}}_array_bsearch(desc_arr, e->{{pargs.element.field}});

				if(!desc)
					goto out_err;

				e->{{pargs.element.field}} = {{pargs.description.type_name}}_array_index(desc_arr, desc);
			}
		}
	}

	return 0;

out_err:
	AM_IOERR_RET1(ctx, AM_IOERR_POSTPROCESS,
		      "Could not find {{desctype.entity}} for {{eltype.entity}} with {{pargs.element.field_entity}} %" {{pargs.element.field_fmt}} ".",
		      e->{{pargs.element.field}});
}
