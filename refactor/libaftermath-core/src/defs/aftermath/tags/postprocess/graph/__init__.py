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

from aftermath.util import enforce_type
from aftermath.tags import FunctionTag, \
    TagWithSteps, \
    TemplatedGenerateFunctionTag
from aftermath.types import Field

import aftermath

class CollectGraphRootsFunction(FunctionTag):
    """A function that traverses an array of instances of a graph node type and adds
    pointers to all instances without a parent to a second array.
    """

    def __init__(self, function_name = None):
        super(CollectGraphRootsFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_collect_roots")

class GenerateCollectGraphRootsFunction(CollectGraphRootsFunction,
                                        TemplatedGenerateFunctionTag):
    """Generate a CollectGraphRootsFunction

    `parent_field` is the field that determines whether a node has a parent. The
    node is added to the target array as a root if the parent is NULL.

    `target_array_type_name` is a string indicating the name of the type of the
    array that stores the roots.

    `target_array_ident` is the array identifier for the target array. The
    target array must be a per-trace array.

    """

    def __init__(self,
                 parent_field,
                 target_array_struct_name,
                 target_array_ident,
                 function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.postprocess.graph.CollectGraphRootsFunction)
        CollectGraphRootsFunction.__init__(self, function_name = function_name)

        enforce_type(parent_field, Field)
        enforce_type(target_array_struct_name, str)
        enforce_type(target_array_ident, str)

        self.__parent_field = parent_field
        self.__target_array_struct_name = target_array_struct_name
        self.__target_array_ident = target_array_ident

    def getTargetArrayStructName(self):
        return self.__target_array_struct_name

    def getTargetArrayIdent(self):
        return self.__target_array_ident

    def getParentField(self):
        return self.__parent_field
