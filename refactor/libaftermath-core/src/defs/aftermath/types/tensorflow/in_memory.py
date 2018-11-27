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

from aftermath.types import TypeList, Field, FieldList
from aftermath.types.in_memory import InMemoryCompoundType
from aftermath import tags
import aftermath.types.base

am_tensorflow_node = InMemoryCompoundType(
    name = "am_tensorflow_node",
    entity = "Tensorflow node",
    comment = "A tensorflow node (e.g., an operation or a constant)",
    ident = "am::tensorflow::node",

    fields = FieldList([
        Field(
            name = "name",
            type = aftermath.types.base.am_string,
            comment = "Name of this node")]))

################################################################################

am_tensorflow_node_execution = InMemoryCompoundType(
    name = "am_tensorflow_node_execution",
    entity = "on-disk Tensorflow node execution",
    comment = "An Tensorflow node execution",
    ident = "am::tensorflow::node_execution",

    fields = FieldList([
        Field(
            name = "node",
            type = am_tensorflow_node,
            is_pointer = True,
            comment = "Node that was executed"),
        Field(
            name = "interval",
            type = aftermath.types.in_memory.am_interval,
            comment = "Interval of the execution")]))

################################################################################

all_types = TypeList([
    am_tensorflow_node,
    am_tensorflow_node_execution,
])

aftermath.config.addMemTypes(*all_types)
