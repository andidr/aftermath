# Author: Andi Drebes <andi@drebesium.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
# USA.

from aftermath.tags import FunctionTag, \
    GenerateFunctionTag, \
    TemplatedGenerateFunctionTag, \
    Compound, \
    FunctionTagStep, \
    TagWithSteps
from aftermath.tags.process import GenerateProcessFunction
from aftermath.util import enforce_type
from aftermath.types import Field, Type, CompoundType, TypeList, FieldList
import aftermath

################################################################################

def _build_canonical_field_map(fieldmap, dsk_type, mem_type):
    """Returns a field map generated from the passed field map, in which strings
    referencing field names have been replaced with their corresponding
    aftermath.types.Field instances from dsk_type and mem_type
    """

    ret = []

    for entry in fieldmap:
        if isinstance(entry, str):
            dsk_field = entry
            mem_field = entry
        elif isinstance(entry, tuple):
            if len(entry) != 2:
                raise Exception("Field map entry must be a pair or a string")

            dsk_field = entry[0]
            mem_field = entry[1]

        if isinstance(dsk_field, str):
            dsk_field = dsk_type.getFields().getFieldByName(dsk_field)
        elif isinstance(dsk_field, Field):
            pass
        else:
            raise Exception("On-disk field specification must be a Field " +\
                            "instance or a string")

        if isinstance(mem_field, str):
            mem_field = mem_type.getFields().getFieldByName(mem_field)
        elif isinstance(mem_field, Field):
            pass
        else:
            raise Exception("In-memory field specification must be a Field "+\
                            "instance or a string")

        ret.append((dsk_field, mem_field))

    return ret

################################################################################

class ConversionFunction(FunctionTag):
    """Indicates that the tagged on-disk type has a conversion function
    converting to an in-memory representation"""

    def __init__(self, dsk_type, mem_type, function_name = None, fieldmap = None,
                 strict_matching = False):
        """`dsk_type` is the on-disk compound type that is transformed into and
        in memory type.

        `mem_type` is the in-memory compound type the on-disk type is transformed
        into.

        `function_name` specified the name of the conversion function generated
        by a template. If none, the name is derived automatically.

        `fieldmap` is a list of pairs of aftermath.types.Field (or pairs of
        strings referring to field names) with the first entry being a field of
        the on-disk type and the second entry being a field of the in-memory
        type indicating how the data types relate or strings referring to a
        field name present in both structures. If a None value has been
        specified for the field map, fields with the same name present in both
        structures are mapped to each other.

        If `strict_matching` is True and the mapping is generated automatically,
        every field of the on-disk data structure must have a counterpart in the
        in-memory structure with the same name.
        """

        enforce_type(dsk_type, Type)
        enforce_type(mem_type, Type)

        enforce_type(function_name, [str, type(None)])
        enforce_type(strict_matching, bool)

        if fieldmap:
            enforce_type(dsk_type, CompoundType)
            enforce_type(mem_type, CompoundType)

        if not fieldmap is None:
            enforce_type(fieldmap, list)
            fieldmap = _build_canonical_field_map(fieldmap, dsk_type, mem_type)
        else:
            fieldmap = []
            dsk_fields = dsk_type.getFields()
            mem_fields = mem_type.getFields()

            for dsk_field in dsk_fields:
                if mem_fields.hasFieldByName(dsk_field.getName()):
                    mem_field = mem_fields.getFieldByName(dsk_field.getName())
                    fieldmap.append((dsk_field, mem_field))
                elif strict_matching:
                    raise Exception("Could not find matching field named " + \
                                    "'" + dsk_field.getName() + "' in type " + \
                                    "'" + mem_type.getName() + "'")

        if fieldmap != []:
            if not dsk_type.hasTag(Compound) or not mem_type.hasTag(Compound):
                raise Exception("Both types must be compound types")


        self.__dsk_type = dsk_type
        self.__mem_type = mem_type
        self.__fieldmap = fieldmap

        super(ConversionFunction, self).__init__(function_name = function_name,
                                                 default_suffix = "_to_mem")

    def getDskType(self):
        return self.__dsk_type

    def getMemType(self):
        return self.__mem_type

    def getFieldMap(self):
        return self.__fieldmap

class GenerateConversionFunction(GenerateFunctionTag, ConversionFunction):
    """Generate a ConversionFunction"""

    def __init__(self, *args, **kwargs):
        ConversionFunction.__init__(self, *args, **kwargs)

################################################################################

class PostConversionFunction(FunctionTag):
    """A post-conversion function is called right after conversion from on-disk to
    in-memory format. A pointer to the on-disk data structure is passed as the
    first argument and a pointer to the in-memory data structure is passed as
    the second argument to the function.

    The address of the in-memory structure is temporary and might change while
    the trace is loaded.
    """
    def __init__(self, function_name = None):
        super(PostConversionFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_post_conversion")

class GeneratePostConversionFunction(TemplatedGenerateFunctionTag,
                                     PostConversionFunction,
                                     TagWithSteps):
    """Generate a PostConversionFunction"""

    def __init__(self, function_name = None):
        PostConversionFunction.__init__(self, function_name = function_name)
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.PostConversionFunction)
        TagWithSteps.__init__(self)

################################################################################

class PerTraceArrayFunction(FunctionTag):
    """Converts an on-disk data structure into its in-memory representation using
    a conversion function and stores the result in the per-trace array indicated
    by the array identifier of the in-memory type.

    For the data model @see
    aftermath.tags.mem.store.pertracearray.GenerateAppendFunction.
    """

    def __init__(self, function_name = None):
        super(PerTraceArrayFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_convert_store_per_trace_array")

class GeneratePerTraceArrayFunction(TemplatedGenerateFunctionTag,
                                    PerTraceArrayFunction):
    """Generate a PerTraceArrayFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.PerTraceArrayFunction)

        PerTraceArrayFunction.__init__(self, function_name = function_name)

def add_per_trace_array_tags(dsk_type,
                             mem_type,
                             trace_array_ident = None,
                             trace_array_struct_name = None,
                             conversion_fun_tag = None):
    """Adds all tags to the on-disk type `dsk_type` and the in-memory type
    `mem_type` to automatically generate functions that use the tag
    `conversion_fun_tag` to convert an instance of the on-disk type into its
    in-memory representation and append the result to the appropriate per-trace
    array.

    `trace_array_ident` is the array identifier for the target array. If None,
    the array identifier of the in-memory data structure is used.

    `trace_array_struct_name` is the name of the type representing the target
    per-trace array. If None, this name is derived by appending "_array" to the
    name of the in-memory type.

    `conversion_fun_tag` is an instance of a tag for conversion function that is
    called for the conversion from on-disk to in-memory format. If None, this
    tag (and thus the conversion function) is created automatically.
    """

    enforce_type(dsk_type, CompoundType)
    enforce_type(mem_type, CompoundType)
    enforce_type(trace_array_ident, [str, type(None)])
    enforce_type(trace_array_struct_name, [str, type(None)])
    enforce_type(conversion_fun_tag, [GenerateConversionFunction, type(None)])

    # Default GenerateConversionFunction tag
    if conversion_fun_tag is None:
        conversion_fun_tag = GenerateConversionFunction(dsk_type, mem_type)

    tracearr_tag = GeneratePerTraceArrayFunction()

    # Append
    append_tag = aftermath.tags.mem.store.pertracearray.GenerateAppendFunction(
        trace_array_ident = trace_array_ident,
        trace_array_struct_name = trace_array_struct_name
    )
    mem_type.addTags(append_tag)

    # Convert and append
    dsk_type.addTags(conversion_fun_tag, tracearr_tag)

    process_tag = dsk_type.getOrAddTagInheriting(GenerateProcessFunction)
    process_tag.addStep(FunctionTagStep(tracearr_tag))

################################################################################

class PerEventCollectionArrayFunction(FunctionTag):
    """Converts an on-disk data structure into its in-memory representation using a
    conversion function and stores the result in the appropriate
    per-event-collection array.

    For the data model @see
    aftermath.tags.mem.store.GeneratePerEventCollectionArrayAppendFunction.
    """

    def __init__(self, function_name = None):
        super(PerEventCollectionArrayFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_convert_store_per_event_collection_array")

class GeneratePerEventCollectionArrayFunction(TemplatedGenerateFunctionTag,
                                              PerEventCollectionArrayFunction):
    """Generate a PerEventCollectionArrayFunction"""

    def __init__(self,
                 event_collection_id_dsk_field,
                 function_name = None):
        """`event_collection_id_dsk_field` is a Field of the on-disk type that
        contains the ID of the target event collection.
        """

        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.PerEventCollectionArrayFunction)

        PerEventCollectionArrayFunction.__init__(
            self,
            function_name = function_name)

        enforce_type(event_collection_id_dsk_field, Field)

        self.__event_collection_id_dsk_field = event_collection_id_dsk_field

    def getEventCollectionDskIDField(self):
        return self.__event_collection_id_dsk_field

def add_per_event_collection_tags(dsk_type,
                                  mem_type,
                                  event_collection_id_dsk_field,
                                  event_array_ident = None,
                                  event_array_struct_name = None,
                                  conversion_fun_tag = None):
    """Adds all tags to the on-disk type `dsk_type` and the in-memory type
    `mem_type` to automatically generate functions that use the tag
    `conversion_fun_tag` to convert an instance of the on-disk type into its
    in-memory representation and append the result to the appropriate
    per-event-collection array.

    `event_collection_id_dsk_field` is the Field of `dsk_type` that contains the
    ID of the target event collection.

    `event_array_ident` is the array identifier for the target array of the
    event collection. If None, the array identifier of the in-memory data
    structure is used.

    `event_array_struct_name` is the name of the type representing the target
    per-event-collection array. If None, this name is derived by appending
    "_array" to the name of the in-memory type.

    `conversion_fun_tag` is an instance of a tag for conversion function that is
    called for the conversion from on-disk to in-memory format. If None, this
    tag (and thus the conversion function) is created automatically.

    """

    enforce_type(dsk_type, CompoundType)
    enforce_type(mem_type, CompoundType)
    enforce_type(event_collection_id_dsk_field, [Field, str])
    enforce_type(event_array_ident, [str, type(None)])
    enforce_type(event_array_struct_name, [str, type(None)])
    enforce_type(conversion_fun_tag, [GenerateConversionFunction, type(None)])

    # Get Field instance if string passed
    if isinstance(event_collection_id_dsk_field, str):
        event_collection_id_dsk_field = dsk_type.getFields().getFieldByName(event_collection_id_dsk_field)

    # Default GenerateConversionFunction tag
    if conversion_fun_tag is None:
        conversion_fun_tag = GenerateConversionFunction(dsk_type, mem_type)

    # Append
    append_tag = aftermath.tags.mem.store.pereventcollectionarray.GenerateAppendFunction(
        event_array_ident = event_array_ident,
        event_array_struct_name = event_array_struct_name
    )
    mem_type.addTags(append_tag)

    # Convert and append
    per_ecoll_tag = GeneratePerEventCollectionArrayFunction(event_collection_id_dsk_field)
    dsk_type.addTags(conversion_fun_tag, per_ecoll_tag)

    process_tag = dsk_type.getOrAddTagInheriting(GenerateProcessFunction)
    process_tag.addStep(FunctionTagStep(per_ecoll_tag))

################################################################################

class PerEventCollectionSubArrayFunction(FunctionTag):
    """Converts an on-disk data structure into its in-memory representation using a
    conversion function and stores the result in the appropriate
    per-event-collection subarray.

    For the data model @see
    aftermath.tags.mem.store.GeneratePerEventCollectionSubArrayAppendFunction.
    """

    def __init__(self, function_name = None):
        super(PerEventCollectionSubArrayFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_store_per_event_collection_sub_array")

class GeneratePerEventCollectionSubArrayFunction(TemplatedGenerateFunctionTag,
                                                 PerEventCollectionSubArrayFunction):
    """Generate a PerEventCollectionSubArrayFunction"""

    def __init__(self,
                 event_collection_id_dsk_field,
                 event_id_dsk_field,
                 function_name = None):

        """`event_collection_id_dsk_field` is a Field of the on-disk type that
        contains the ID of the target event collection.

        `event_id_dsk_field` is a Field of the on-disk type that contains the
        numerical ID of the target subarray for the event collection.
        """

        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.PerEventCollectionSubArrayFunction)
        PerEventCollectionSubArrayFunction.__init__(self, function_name = function_name)

        self.__event_collection_id_dsk_field = event_collection_id_dsk_field
        self.__event_id_dsk_field = event_id_dsk_field

    def getEventCollectionIDDskField(self):
        return self.__event_collection_id_dsk_field

    def getEventIDDskField(self):
        return self.__event_id_dsk_field

def add_per_event_collection_sub_array_tags(dsk_type,
                                            mem_type,
                                            event_collection_id_dsk_field,
                                            event_id_dsk_field,
                                            event_collection_array_ident = None,
                                            event_collection_array_struct_name = None,
                                            event_array_struct_name = None,
                                            conversion_fun_tag = None):
    """Adds all tags to the on-disk type `dsk_type` and the in-memory type
    `mem_type` to automatically generate functions that use the tag
    `conversion_fun_tag` to convert an instance of the on-disk type into its
    in-memory representation and append the result to the appropriate
    per-event-collection subarray.

    `event_collection_id_dsk_field` is the Field of `dsk_type` that contains the
    ID of the event collection that contains the per-event-collection array that
    itself contains the target subarray.

    `event_id_dsk_field` is the Field of `dsk_type` that contains the numerical
    ID of the target subarray.

    `event_collection_array_ident` is the array identifier for the target array
    of the event collection. If None, the array identifier of the in-memory data
    structure is used.

    `event_collection_array_struct_name` is the name of the type representing
    the per-event-collection array. If None, this name is derived by appending
    "_array" to `event_array_struct_name`.

    `event_array_struct_name` is the name of the type representing the
    per-event-collection subarray. If None, this name is derived by appending
    "_array" to the name of the in-memory type.

    `conversion_fun_tag` is an instance of a tag for conversion function that is
    called for the conversion from on-disk to in-memory format. If None, this
    tag (and thus the conversion function) is created automatically.

    """

    enforce_type(dsk_type, CompoundType)
    enforce_type(mem_type, CompoundType)
    enforce_type(event_collection_id_dsk_field, [Field, str])
    enforce_type(event_id_dsk_field, [Field, str])
    enforce_type(event_collection_array_struct_name, [str, type(None)])
    enforce_type(conversion_fun_tag, [GenerateConversionFunction, type(None)])

    # Get Field instances if string passed
    if isinstance(event_collection_id_dsk_field, str):
        event_collection_id_dsk_field = dsk_type.getFields().getFieldByName(
            event_collection_id_dsk_field)

    if isinstance(event_id_dsk_field, str):
        event_id_dsk_field = dsk_type.getFields().getFieldByName(event_id_dsk_field)

    # Default GenerateConversionFunction tag
    if conversion_fun_tag is None:
        conversion_fun_tag = GenerateConversionFunction(dsk_type, mem_type)

    # Append
    append_tag = aftermath.tags.mem.store.pereventcollectionsubarray.GenerateAppendFunction(
        sub_array_id_type = event_id_dsk_field.getType(),
        event_collection_array_ident = event_collection_array_ident,
        event_collection_array_struct_name = event_collection_array_struct_name,
        event_array_struct_name = event_array_struct_name
    )
    mem_type.addTags(append_tag)

    # Convert and append
    subarr_tag = GeneratePerEventCollectionSubArrayFunction(
                 event_collection_id_dsk_field = event_collection_id_dsk_field,
                 event_id_dsk_field = event_id_dsk_field)

    dsk_type.addTags(conversion_fun_tag, subarr_tag)

    process_tag = dsk_type.getOrAddTagInheriting(GenerateProcessFunction)
    process_tag.addStep(FunctionTagStep(subarr_tag))

################################################################################

class SortMetaStructArrayFunction(FunctionTag):
    """Sorts an array of metadata structures"""

    def __init__(self, function_name = None):
        super(SortMetaStructArrayFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_sort_array")

class GenerateSortMetaStructArrayFunction(TemplatedGenerateFunctionTag,
                                          SortMetaStructArrayFunction):
    """Generate a SortMetaStructArrayFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.SortMetaStructArrayFunction)
        SortMetaStructArrayFunction.__init__(self, function_name = function_name)

################################################################################

class MatchAllMetaStructArraysFunction(FunctionTag):
    """Matches all elements of a set of arrays of source data structures with the
    corresponding elements of a set of arrays of target data structures based on
    their associated meta data arrays.

    The matching for an element ist done as follows:

    (1) A search function finds the element in the meta-data array "MB" of the
        target array that corresponds to the element in the meta-data array "MA"
        of the source array.

    (2) The code retrieves a pointer to the actual source data structure, by
        dereferencing the source array "A" associated to "MA" using the same
        index as within "MA".

    (3) The code retrieves a pointer to the actual target data structure, by
        dereferencing the source array "B" associated to "MB" using the field
        "idx" of the structure from "MB" (it must use the field "idx" and cannot
        use directly the index of the structure within "MB", since the elements
        in "MB" are sorted and the indexes might not be identical anymore).

    (4) The field with the pointer in the source data structure is set to the
        address of the element in "B".

    struct                                  struct
    <mem_source_t>__meta_<pointer_name> {   <mem_source_t>__meta_<pointer_name> {
      <dsk_source_field> id;                    <dsk_source_field> id;
    };                                          size_t idx;
                                             };

    Array "MA" (with meta-data for           Array "MB" with meta-data for target
    source (with matching array indexes)     (with matching array indexes)
    +--+--+--+--+--+--+--+--+                +--+--+--+--+--+--+--+--+
    |  |  |  |  |  |  |  |  |                |  |  |  |  |  |  |  |  |
    +--+--+--+--+--+--+--+--+                +--+--+--+--+--+--+--+--+
              ||                                    ^|
              ||               (1)                  ||
              | \-----------------------------------/\------
              |                                             \  (3) idx
           (2)|  -------------------------------------------\\
              | /              (4)                           \|
              ||                                             ||
              v|                                             vv
    +--+--+--+--+--+--+--+--+                +--+--+--+--+--+--+--+--+
    |  |  |  |  |  |  |  |  |                |  |  |  |  |  |  |  |  |
    +--+--+--+--+--+--+--+--+                +--+--+--+--+--+--+--+--+
    Array "A" with source structures         Array "B" with target structures
    """

    def __init__(self, function_name = None):
        super(MatchAllMetaStructArraysFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_match_all_meta_struct_arrays")

class GenerateMatchAllMetaStructArraysFunction(TemplatedGenerateFunctionTag,
                                               MatchAllMetaStructArraysFunction):
    """Generate a MatchAllMetaStructArraysFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.MatchAllMetaStructArraysFunction)

        MatchAllMetaStructArraysFunction.__init__(self, function_name = function_name)

################################################################################

class AddToAllMetaStructArraysFunction(FunctionTag):
    """Adds one entry to each array of meta-data structures for one instance of a
    data structure."""

    def __init__(self, function_name = None):
        super(AddToAllMetaStructArraysFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_add_to_all_meta_struct_arrays")

class GenerateAddToAllMetaStructArraysFunction(TemplatedGenerateFunctionTag,
                                               AddToAllMetaStructArraysFunction):
    """Generate a AddToAllMetaStructArraysFunction"""

    def __init__(self, dsk_type, mem_type, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.tomem.AddToAllMetaStructArraysFunction)

        AddToAllMetaStructArraysFunction.__init__(self, function_name = function_name)

        enforce_type(dsk_type, Type)
        enforce_type(mem_type, Type)
        self.__dsk_type = dsk_type
        self.__mem_type = mem_type

    def getDskType(self):
        return self.__dsk_type

    def getMemType(self):
        return self.__mem_type
