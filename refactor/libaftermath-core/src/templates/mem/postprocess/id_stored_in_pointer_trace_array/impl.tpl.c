{%- set pargs = postprocess.args -%}
{%- set linking_type = mem.find(pargs.linking_element.type_name) %}
{%- set linked_type = mem.find(pargs.linked_element.type_name) %}

{% include "fnproto.tpl.h" %}
{
	struct {{linking_type.name}}_array* linking_element_arr;
	struct {{linked_type.name}}_array* linked_element_arr;
	{{linking_type.c_type}}* linking_element;
	{{linked_type.c_type}}* linked_element;
	uintptr_t uval;
	typeof(linked_element->{{pargs.linked_element.id_field}}) id;

	linking_element_arr = &ctx->trace->{{pargs.linking_element.trace_array_field}};
	linked_element_arr = &ctx->trace->{{pargs.linked_element.trace_array_field}};

	assert(sizeof(void*) <= sizeof(linked_element->{{pargs.linked_element.id_field}}));

	for(linking_element = &linking_element_arr->elements[0];
	    linking_element != &linking_element_arr->elements[linking_element_arr->num_elements];
	    linking_element++)
	{
		uval = (uintptr_t)linking_element->{{pargs.linking_element.field}};
		id = uval;

		if(!(linked_element = am_openstream_task_type_array_bsearch(
			     linked_element_arr, id)))
		{
			AM_IOERR_RET1(ctx, AM_IOERR_POSTPROCESS,
				      "Could not find {{linked_type.entity}} with ID %" PRIuPTR " "
				      "for {{linking_type.entity}}.", uval);
		}
	}

	return 0;
}
