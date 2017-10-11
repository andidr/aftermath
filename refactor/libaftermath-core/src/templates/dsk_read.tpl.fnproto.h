{%- if t.is_frame -%}
static inline int {{t.name}}_read_skip_type(struct am_io_context* ctx, {{t.c_type}}* f);
{% elif "dsk_read" in t.defs -%}
static inline int {{t.name}}_read(struct am_io_context* ctx, {{t.c_type}}* f);
{% endif -%}
