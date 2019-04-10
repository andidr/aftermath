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

am_dsk_openstream_task_type = Frame(
    name = "am_dsk_openstream_task_type",
    entity = "on-disk OpenStream task type",
    comment = "An OpenStream task type",
    fields = FieldList([
        Field(
            name = "type_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this task type"),
        Field(
            name = "name",
            field_type = aftermath.types.on_disk.am_dsk_string,
            comment = "Name of this task (e.g., symbol in the executable)"),
        Field(
            name = "source",
            field_type = aftermath.types.on_disk.am_dsk_source_location,
            comment = "Location of the source code for this task type")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openstream_task_type,
    aftermath.types.openstream.in_memory.am_openstream_task_type)

################################################################################

am_dsk_openstream_task_instance = Frame(
    name = "am_dsk_openstream_task_instance",
    entity = "on-disk OpenStream task instance",
    comment = "An instance of an OpenStream task",

    fields = FieldList([
        Field(
            name = "type_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the task type for this instance"),
        Field(
            name = "instance_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this task instance")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openstream_task_instance,
    aftermath.types.openstream.in_memory.am_openstream_task_instance)

relations.join.make_join(
    dsk_src_field = am_dsk_openstream_task_instance.getFields().getFieldByName("type_id"),
    dsk_target_field = am_dsk_openstream_task_type.getFields().getFieldByName("type_id"),
    mem_ptr_field = aftermath.types.openstream.in_memory.am_openstream_task_instance.getFields().getFieldByName("task_type"),
    mem_target_type = aftermath.types.openstream.in_memory.am_openstream_task_type,
    null_allowed = False)

################################################################################

am_dsk_openstream_task_period = EventFrame(
    name = "am_dsk_openstream_task_period",
    entity = "on-disk OpenStream task execution period",
    comment = "An OpenStream task execution period (contiguous interval of " + \
              "execution of a task)",
    fields = FieldList([
        Field(
            name = "instance_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the OpenStream task instance this " + \
                      "period accounts for"),
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_openstream_task_period,
    aftermath.types.openstream.in_memory.am_openstream_task_period,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_openstream_task_period.getFields().getFieldByName("instance_id"),
    dsk_target_field = am_dsk_openstream_task_instance.getFields().getFieldByName("instance_id"),
    mem_ptr_field = aftermath.types.openstream.in_memory.am_openstream_task_period.getFields().getFieldByName("task_instance"),
    mem_target_type = aftermath.types.openstream.in_memory.am_openstream_task_instance,
    null_allowed = False
)

################################################################################

all_types = TypeList([
    am_dsk_openstream_task_type,
    am_dsk_openstream_task_instance,
    am_dsk_openstream_task_period
])

aftermath.config.addDskTypes(*all_types)
