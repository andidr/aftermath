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
from aftermath import relations
from aftermath import tags
from aftermath.types.on_disk import Frame, EventFrame
import aftermath.types.on_disk

am_dsk_tensorflow_node = Frame(
    name = "am_dsk_tensorflow_node",
    entity = "on-disk Tensorflow node",
    comment = "An Tensorflow node",
    fields = FieldList([
        Field(
            name = "id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this node"),
        Field(
            name = "name",
            type = aftermath.types.on_disk.am_dsk_string,
            comment = "Name of this node")]))

tags.dsk.tomem.add_per_trace_array_tags(
   am_dsk_tensorflow_node,
   aftermath.types.tensorflow.in_memory.am_tensorflow_node)

################################################################################

am_dsk_tensorflow_node_execution = EventFrame(
    name = "am_dsk_tensorflow_node_execution",
    entity = "on-disk Tensorflow node execution",
    comment = "An Tensorflow node execution",

    fields = FieldList([
        Field(
            name = "node_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the node that was executed"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_tensorflow_node_execution,
    aftermath.types.tensorflow.in_memory.am_tensorflow_node_execution,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_tensorflow_node_execution.getFields().getFieldByName("node_id"),
    dsk_target_field = am_dsk_tensorflow_node.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.tensorflow.in_memory.am_tensorflow_node_execution.getFields().getFieldByName("node"),
    mem_target_type = aftermath.types.tensorflow.in_memory.am_tensorflow_node,
    null_allowed = False
)


################################################################################

all_types = TypeList([
    am_dsk_tensorflow_node,
    am_dsk_tensorflow_node_execution,
])

aftermath.config.addDskTypes(*all_types)
