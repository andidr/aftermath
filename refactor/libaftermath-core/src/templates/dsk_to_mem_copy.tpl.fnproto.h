{% set dsktype = t %}
{% set memtype = mem.find_same_dsk(dsktype.name) -%}
static inline int {{dsktype.name}}_to_mem(struct am_io_context* ctx, {{dsktype.c_type}}* dsk, {{memtype.c_type}}* out);
