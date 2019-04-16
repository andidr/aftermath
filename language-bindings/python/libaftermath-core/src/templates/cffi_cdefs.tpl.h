struct am_typed_array_generic {
	size_t num_elements;
	size_t num_free;
	size_t num_prealloc;
	void* elements;
};

{% for (array_name, element_type) in array_element_types.items() %}
{{element_type}}* am_py_{{array_name}}_get_element(struct am_typed_array_generic* a, size_t i);
{% endfor %}

size_t am_py_generic_array_get_num_elements(struct am_typed_array_generic* a);

struct am_trace;
struct am_event_collection;

struct am_trace* am_py_trace_load(const char* filename);
void am_py_trace_destroy_and_free(struct am_trace* t);
struct am_typed_array_generic*
am_py_trace_find_trace_array(struct am_trace* t, const char* type);

struct am_typed_array_generic*
am_py_event_collection_find_event_array(struct am_event_collection* ecoll,
					const char* ident);

struct am_typed_array_generic*
am_py_trace_get_event_collections(struct am_trace* t);

size_t am_py_trace_get_num_event_collections(struct am_trace* t);

struct am_event_collection*
am_py_event_collection_array_get_element(struct am_typed_array_generic* a,
					 size_t i);
