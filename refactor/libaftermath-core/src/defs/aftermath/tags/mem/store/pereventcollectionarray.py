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

from aftermath.tags import FunctionTag, TemplatedGenerateFunctionTag
from aftermath.util import enforce_type
import aftermath

class DestroyAllArraysFunction(FunctionTag):
    """Iterates over all event collections and destroys all arrays which have
    the array ident of the type this tag is associated to.
    """

    def __init__(self, function_name = None):
        super(DestroyAllArraysFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_per_trace_array_destroy_all_arrays")

class GenerateDestroyAllArraysFunction(DestroyAllArraysFunction,
                                       TemplatedGenerateFunctionTag):
    """Generate a DestroyAllArraysFunction"""

    def __init__(self, function_name = None):
        DestroyAllArraysFunction.__init__(self, function_name = function_name)
        TemplatedGenerateFunctionTag.__init__(
            self, template_type = aftermath.templates.mem.store.pereventcollectionarray.DestroyAllArraysFunction)

class AppendFunction(FunctionTag):
    """Appends an instance of an in-memory type to the per-event-collection array
    with the array ident of the type this tag is associated to. If the array
    does not exist, it is created.
    """

    def __init__(self, function_name = None):
        super(AppendFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_per_event_collection_array_append")

class AppendAndSetIndexFunction(FunctionTag):
    """Same as AppendFunction, but sets the field 'idx' of the added instance to
    the index within the array it has been added to."""

    def __init__(self, function_name = None):
        super(AppendAndSetIndexFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_per_event_collection_array_append_and_set_index")

class BaseGenerateAppendFunction(TemplatedGenerateFunctionTag):
    """Base class for the tags GenerateAppendFunction and
    GenerateAppendAndSetIndexFunction, grouping common functionality between the
    two classes.
    """

    def __init__(self,
                 template_type,
                 event_array_ident = None,
                 event_array_struct_name = None):
        """The string `event_array_ident` is the string identifier of the event
        collection's event array (e.g., "am::generic::state_event"). If this
        value is not specified, the string identifier is determined by using the
        string identifier published by the structure that contains the in-memory
        representation of the on-disk data structure.

        The string `event_array_struct_name` specifies the name of the data
        structure of the per-event collection array. If not specified, it is
        automatically derived from the in-memory type name by appending "_array"

        +--------------------+
        | Trace              |
        +--------------------+
        |                    |
        | Event collections  |--- [ event_collection_id_dsk_field ] --+
        |                    |                                        |
        |                    |                           +----------------------+
        |                    |                           | Event collection     |
        +--------------------+                           +----------------------+
                                                         |                      |
                 +------- [ event_array_ident ] ---------| Event arrays         |
                 |                                       |                      |
        +----------------------------+                   |                      |
        | event_array_struct_name    |                   +----------------------+
        +----------------------------+
        | in-memory event type       |
        | in-memory event type       |
        | in-memory event type       |
        | ...                        |
        | in-memory event type       |
        |                            | <--- insert position
        +----------------------------+
        """

        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = template_type)

        enforce_type(event_array_ident, [str, type(None)])
        enforce_type(event_array_struct_name, [str, type(None)])

        self.__event_array_ident = event_array_ident
        self.__event_array_struct_name = event_array_struct_name

    def getEventArrayIdent(self):
        return self.__event_array_ident

    def getEventArrayStructName(self):
        return self.__event_array_struct_name

class GenerateAppendFunction(BaseGenerateAppendFunction, AppendFunction):
    """Generate an AppendFunction"""

    def __init__(self,
                 function_name = None,
                 event_array_ident = None,
                 event_array_struct_name = None):
        """See BaseGenerateAppendFunction.__init__"""

        AppendFunction.__init__(
            self,
            function_name = function_name)

        BaseGenerateAppendFunction.__init__(
            self,
            template_type = aftermath.templates.mem.store.pereventcollectionarray.AppendFunction,
            event_array_ident = event_array_ident,
            event_array_struct_name = event_array_struct_name)

class GenerateAppendAndSetIndexFunction(BaseGenerateAppendFunction,
                                        AppendAndSetIndexFunction):
    """Generate an AppendAndSetIndexFunction"""

    def __init__(self,
                 function_name = None,
                 event_array_ident = None,
                 event_array_struct_name = None):
        """See BaseGenerateAppendFunction.__init__"""

        AppendAndSetIndexFunction.__init__(
            self,
            function_name = function_name)

        BaseGenerateAppendFunction.__init__(
            self,
            template_type = aftermath.templates.mem.store.pereventcollectionarray.AppendAndSetIndexFunction,
            event_array_ident = event_array_ident,
            event_array_struct_name = event_array_struct_name)
