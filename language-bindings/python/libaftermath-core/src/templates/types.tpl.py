from aftermath.core._aftermath_core import ffi

{% set mem_types = aftermath.config.getMemTypes() %}
{% set base_types = aftermath.config.getBaseTypes() %}
{% set builtin_types = aftermath.types.builtin.all_types %}

# Array that is defined by a simple pointer of a compound struct and *not* as a
# typed array
class WrappedElementAdHocArray(object):
    def __init__(self, ptr_base, num_elements, element_wrapper_type, owner):
        self.__ptr_base = ptr_base
        self.__num_elements = num_elements
        self.__element_wrapper_type = element_wrapper_type
        self.__owner = owner

    def __getitem__(self, idx):
        if idx >= self.__num_elements:
            raise IndexError()
        else:
            return self.__element_wrapper_type(self.__ptr_base[idx], self)

    def __len__(self):
        self.__num_elements

    def __iter__(self):
        from aftermath.core.array import DefaultIterator
        return DefaultIterator(self)

{% for t in mem_types.topologicalSort().filterByTag(aftermath.tags.Compound) -%}
# Auto-generated wrapper type for {{t.getName()}}
class {{t.getName()|stripped_camel_case}}(object):
    """{{t.getComment()}}"""

    def __init__(self, cffi_element, owner):
        self.__cffi_element = cffi_element
        self.__owner = owner
{# #}
    {%- for field in t.getFields() %}
    {%- set ftype = field.getType() %}
    @property
    def {{field.getName()}}(self):
        """{{field.getComment()}}"""

        {% if not field.isPointer() -%}
            {%- if ftype == aftermath.types.builtin.charp or
             ftype == aftermath.types.base.am_string -%}
        return str(ffi.string(self.__cffi_element.{{field.getName()}}))
            {%- elif mem_types.hasType(ftype) and ftype.isCompound()-%}
        return {{ftype.getName()|stripped_camel_case}}(self.__cffi_element.{{field.getName()}}, self)
            {%- elif not ftype.isCompound() or builtin_types.hasType(ftype) or base_types.hasType(ftype) -%}
        return self.__cffi_element.{{field.getName()}}
            {%- endif -%}
        {%- else %}
            {%- if field.isArray() -%}
        return WrappedElementAdHocArray(
            self.__cffi_element.{{field.getName()}},
            self.__cffi_element.{{field.getArrayNumElementsFieldName()}},
            self.__class__,
            self)
            {%- else -%}
        cffi_field = self.__cffi_element.{{field.getName()}}

        if cffi_field == ffi.NULL:
            return None
{# #}
                {%- if ftype == aftermath.types.builtin.charp or
                 ftype == aftermath.types.base.am_string %}
        return str(ffi.string(cffi_field[0]))
                {%- elif mem_types.hasType(t) %}
        return {{ftype.getName()|stripped_camel_case}}(cffi_field[0], self)
                {%- elif builtin_types.hasType(ftype) or base_types.hasType(ftype) %}
        return cffi_field[0]
                {%- endif -%}
            {%- endif %}
        {%- endif %}
{# #}
    {%- endfor %}
{# #}
{%- endfor %}
