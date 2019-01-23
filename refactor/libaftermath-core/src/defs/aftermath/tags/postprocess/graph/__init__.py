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

class CheckRootCountFunction(FunctionTag):
    """A function that checks if the root count for a graph is correct."""

    def __init__(self, function_name = None):
        super(CheckRootCountFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_check_root_count")

class GenerateCheckRootCountFunction(CheckRootCountFunction,
                                     TemplatedGenerateFunctionTag):
    """Generate a CheckGraphRootsFunction

    `node_array_struct_name` is the name of the structure representing arrays of
    nodes.

    `node_array_ident` is the array identifier for arrays of nodes.

    `rootp_array_struct_name` is the name of the structure for the array of
    pointers to root nodes.

    `rootp_array_ident` is the array identifier for the per-trace array of
    pointers to root nodes.

    `min_count` is the minimum expected root count.

    `max_count` is the minimum expected root count.

    `triggers_only_if_at_leats_one_node` is a boolean indicating whether the
    root count check should only be triggered if there is at least one node.
    """

    def __init__(self,
                 node_array_struct_name,
                 node_array_ident,
                 rootp_array_struct_name,
                 rootp_array_ident,
                 min_count,
                 max_count,
                 triggers_only_if_at_leats_one_node = True,
                 function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.postprocess.graph.CheckRootCountFunction)
        CheckRootCountFunction.__init__(self, function_name = function_name)

        enforce_type(node_array_struct_name, str)
        enforce_type(node_array_ident, str)
        enforce_type(rootp_array_struct_name, str)
        enforce_type(rootp_array_ident, str)
        enforce_type(min_count, int)
        enforce_type(max_count, int)
        enforce_type(triggers_only_if_at_leats_one_node, bool)

        if min_count > max_count:
            raise Exception("Maximum count must be greater than or equal to " +
                            "the minimum count")

        self.__node_array_struct_name = node_array_struct_name
        self.__node_array_ident = node_array_ident
        self.__rootp_array_struct_name = rootp_array_struct_name
        self.__rootp_array_ident = rootp_array_ident
        self.__min_count = min_count
        self.__max_count = max_count
        self.__triggers_only_if_at_leats_one_node = triggers_only_if_at_leats_one_node

    def getNodeArrayStructName(self):
        return self.__node_array_struct_name

    def getNodeArrayIdent(self):
        return self.__node_array_ident

    def getRootPointerArrayStructName(self):
        return self.__rootp_array_struct_name

    def getRootPointerArrayIdent(self):
        return self.__rootp_array_ident

    def getMinCount(self):
        return self.__min_count

    def getMaxCount(self):
        return self.__max_count

    def triggersOnlyIfAtLeatsOneNode(self):
        return self.__triggers_only_if_at_leats_one_node
