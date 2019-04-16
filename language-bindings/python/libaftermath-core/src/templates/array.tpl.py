from aftermath.core._aftermath_core import lib
import aftermath.core.types as types

class DefaultIterator(object):
    """A default iterator for an object that implements __getitem__ and
    __len__"""

    def __init__(self, array):
        self.__array = array
        self.__idx = 0

    def next(self):
        try:
            r = self.__array[self.__idx]
        except IndexError:
            raise StopIteration()
        except:
            raise

        self.__idx += 1
        return r

    def __next__(self):
        return self.next()

class Array(object):
    """Wraps a cffi array `arr`

    `num_elements` indicates how many elements the array holds.

    `get_element_function` is a cffi function that returns the i-th element of
    the array when called with `arr` and `i` as the arguments.

    `element_wrapper_type` is the Python type instantiated when an element is
    retrieved via `__getitem__`.

    `owner` is a back reference to the owning object or an object that holds a
    direct or indirect reference to the owner of the cffi object in order to
    prevent deallocation of the cffi object before the end of the life time of
    this instance.
    """
    def __init__(self,
                 arr,
                 num_elements,
                 get_element_fun,
                 element_wrapper_type,
                 owner):
        self.__array = arr
        self.__num_elements = num_elements
        self.__get_element_fun = get_element_fun
        self.__element_wrapper_class = element_wrapper_type
        self.__owner = owner

    def __getitem__(self, idx):
        if idx >= len(self):
            raise IndexError()
        else:
            cffi_item = self.__get_element_fun(self.__array, idx)
            return self.__element_wrapper_class(cffi_item, self)

    def __len__(self):
        return self.__num_elements

    def __iter__(self):
        return DefaultIterator(self)

{% set mem_types = aftermath.config.getMemTypes() %}

{% for (array_name, array_ident) in array_idents.items() -%}
{%- set eltype = mem_types.getTypeByIdent(array_ident) %}
{%- if eltype %}
# Auto-generated array type for typed array {{array_name}}
class {{array_name|stripped_camel_case}}(Array):
    def __init__(self, arr, owner = None):
        Array.__init__(self,
                       arr,
                       lib.am_py_generic_array_get_num_elements(arr),
                       lib.am_py_{{array_name}}_get_element,
                       types.{{eltype.getName()|stripped_camel_case}},
                       owner)
{% endif %}
{% endfor %}

_array_maps = {
    {%- for (array_name, array_ident) in array_idents.items() -%}
    {%- set eltype = mem_types.getTypeByIdent(array_ident) %}
    {%- if eltype %}
    "{{array_ident}}" : {{array_name|stripped_camel_case}},
    {%- endif %}
    {%- endfor %}
}

def wrap(ident, arr, owner = None):
    """Returns an Array wrapping the cffi array `arr` if `ident` is a known
    array identifier"""

    if ident in _array_maps.keys():
        return _array_maps[ident](arr, owner)
    else:
        raise Exception("Unknown array type " + str(ident))
