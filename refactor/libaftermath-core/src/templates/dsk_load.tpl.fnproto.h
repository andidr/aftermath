{%- if t.is_frame -%}
static inline int {{t.name}}_load(struct am_io_context* ctx);
{% endif -%}
