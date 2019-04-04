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
    """Destroys all per-trace arrays arrays which have the array ident of the
    type this tag is associated to
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
            self, template_type = aftermath.templates.mem.store.pertracearray.DestroyAllArraysFunction)

class AppendFunction(FunctionTag):
    """Appends an instance of an in-memory type to the per-trace array with the
    array ident of the type this tag is associated to. If the array does not
    exist, it is created.
    """

    def __init__(self, function_name = None):
        super(AppendFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_per_trace_array_append")

class AppendAndSetIndexFunction(FunctionTag):
    """Same as AppendFunction, but sets the field 'idx' of the added instance to
    the index within the array it has been added to."""

    def __init__(self, function_name = None):
        super(AppendAndSetIndexFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_per_trace_array_append_and_set_index")

class BaseGenerateAppendFunction(TemplatedGenerateFunctionTag):
    """Base class for the tags GenerateAppendFunction and
    GenerateAppendAndSetIndexFunction, grouping common functionality between the
    two classes.
    """

    def __init__(self,
                 template_type,
                 trace_array_ident = None,
                 trace_array_struct_name = None):
        """`trace_array_ident` is the string identifier of the per-trace array (e.g.,
        "am::core::state_description"). If not specified, it is determined
        automatically by using the in-memory type's array identifier.

        `trace_array_struct_name` is a string indicating the name of the
        structure that represents the per-trace array (e.g.,
        "am_state_description_array"). If not specified, it is derived by
        extending the in-memory type's name with "_array".

        +--------------------+
        | Trace              |
        +--------------------+
        |                    |
        | Trace arrays       |---- [ trace_array_ident ] --+
        |                    |                             |
        |                    |                       +--------------------------+
        |                    |                       | trace_array_struct_name  |
        +--------------------+                       +--------------------------+
                                                     |                          |
                                                     | in-memory event type     |
                                                     | in-memory event type     |
                                                     | in-memory event type     |
                                                     | ...                      |
                                                     | in-memory event type     |
                                insert position ---> |                          |
                                                     +--------------------------+
        """

        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = template_type)

        enforce_type(trace_array_ident, [str, type(None)])
        enforce_type(trace_array_struct_name, [str, type(None)])

        self.__trace_array_ident = trace_array_ident
        self.__trace_array_struct_name = trace_array_struct_name

    def getTraceArrayIdent(self):
        return self.__trace_array_ident

    def getTraceArrayStructName(self):
        return self.__trace_array_struct_name

class GenerateAppendFunction(BaseGenerateAppendFunction, AppendFunction):
    """Generate an AppendFunction"""

    def __init__(self,
                 function_name = None,
                 trace_array_ident = None,
                 trace_array_struct_name = None):
        """See BaseGenerateAppendFunction.__init__"""

        AppendFunction.__init__(self, function_name = function_name)

        BaseGenerateAppendFunction.__init__(
            self,
            template_type = aftermath.templates.mem.store.pertracearray.AppendFunction,
            trace_array_ident = trace_array_ident,
            trace_array_struct_name = trace_array_struct_name)

class GenerateAppendAndSetIndexFunction(BaseGenerateAppendFunction,
                                        AppendAndSetIndexFunction):
    """Generate an AppendAndSetIndexFunction"""

    def __init__(self,
                 function_name = None,
                 trace_array_ident = None,
                 trace_array_struct_name = None):
        """See BaseGenerateAppendFunction.__init__"""

        AppendAndSetIndexFunction.__init__(self, function_name = function_name)
        BaseGenerateAppendFunction.__init__(
            self,
            template_type = aftermath.templates.mem.store.pertracearray.AppendAndSetIndexFunction,
            trace_array_ident = trace_array_ident,
            trace_array_struct_name = trace_array_struct_name)
