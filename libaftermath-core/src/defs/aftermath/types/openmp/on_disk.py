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

am_dsk_openmp_for_loop_type = Frame(
    name = "am_dsk_openmp_for_loop_type",
    entity = "on-disk OpenMP for loop",
    comment = "An OpenMP for loop type",
    fields = FieldList([
        Field(
            name = "type_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this loop type"),
        Field(
            name = "source",
            type = aftermath.types.on_disk.am_dsk_source_location,
            comment = "Location of the source code for this for loop type"),
        Field(
            name = "addr",
            type = aftermath.types.builtin.uint64_t,
            comment = "Address of the first instruction of the outlined loop body"),
        Field(
            name = "flags",
            type = aftermath.types.builtin.uint32_t,
            comment = "Flags for this loop type")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openmp_for_loop_type,
    aftermath.types.openmp.in_memory.am_openmp_for_loop_type)

################################################################################

am_dsk_openmp_for_loop_instance = Frame(
    name = "am_dsk_openmp_for_loop_instance",
    entity = "on-disk OpenMP for loop instance",
    comment = "An instance of an OpenMP for loop",

    fields = FieldList([
        Field(
            name = "type_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the for loop type for this instance"),
        Field(
            name = "instance_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this for_loop instance"),
        Field(
            name = "lower_bound",
            type = aftermath.types.builtin.uint64_t,
            comment = "First value (included) for this loop instance"),
        Field(
            name = "upper_bound",
            type = aftermath.types.builtin.uint64_t,
            comment = "Last value (included) for this loop instance"),
        Field(
            name = "num_workers",
            type = aftermath.types.builtin.uint32_t,
            comment = "Number of workers that participated in the execution of this loop instance")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openmp_for_loop_instance,
    aftermath.types.openmp.in_memory.am_openmp_for_loop_instance)

relations.join.make_join(
    dsk_src_field = am_dsk_openmp_for_loop_instance.getFields().getFieldByName("type_id"),
    dsk_target_field = am_dsk_openmp_for_loop_type.getFields().getFieldByName("type_id"),
    mem_ptr_field = aftermath.types.openmp.in_memory.am_openmp_for_loop_instance.getFields().getFieldByName("loop_type"),
    mem_target_type = aftermath.types.openmp.in_memory.am_openmp_for_loop_type,
    null_allowed = False)

################################################################################

am_dsk_openmp_iteration_set = Frame(
    name = "am_dsk_openmp_iteration_set",
    entity = "on-disk OpenMP for loop iteration set",
    comment = "A set of iterations executed by a worker",

    fields = FieldList([
        Field(
            name = "instance_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the for loop instance this iteration set belongs to"),
        Field(
            name = "iteration_set_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this for loop iteration set"),
        Field(
            name = "lower_bound",
            type = aftermath.types.builtin.uint64_t,
            comment = "First value (included) for this iteration set"),
        Field(
            name = "upper_bound",
            type = aftermath.types.builtin.uint64_t,
            comment = "Last value (included) for this iteration set")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openmp_iteration_set,
    aftermath.types.openmp.in_memory.am_openmp_iteration_set)

relations.join.make_join(
    dsk_src_field = am_dsk_openmp_iteration_set.getFields().getFieldByName("instance_id"),
    dsk_target_field = am_dsk_openmp_for_loop_instance.getFields().getFieldByName("instance_id"),
    mem_ptr_field = aftermath.types.openmp.in_memory.am_openmp_iteration_set.getFields().getFieldByName("loop_instance"),
    mem_target_type = aftermath.types.openmp.in_memory.am_openmp_for_loop_instance,
    null_allowed = False)

################################################################################

am_dsk_openmp_iteration_period = EventFrame(
    name = "am_dsk_openmp_iteration_period",
    entity = "on-disk OpenMP iteration execution period",
    comment = "An OpenMP iteration period (contiguous interval of " + \
              "execution of an iteration set)",
    fields = FieldList([
        Field(
            name = "iteration_set_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the OpenMP iteration set this " + \
                      "period belongs to"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_openmp_iteration_period,
    aftermath.types.openmp.in_memory.am_openmp_iteration_period,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_openmp_iteration_period.getFields().getFieldByName("iteration_set_id"),
    dsk_target_field = am_dsk_openmp_iteration_set.getFields().getFieldByName("iteration_set_id"),
    mem_ptr_field = aftermath.types.openmp.in_memory.am_openmp_iteration_period.getFields().getFieldByName("iteration_set"),
    mem_target_type = aftermath.types.openmp.in_memory.am_openmp_iteration_set,
    null_allowed = False
)

################################################################################

am_dsk_openmp_task_type = Frame(
    name = "am_dsk_openmp_task_type",
    entity = "on-disk OpenMP task type",
    comment = "An OpenMP task type",
    fields = FieldList([
        Field(
            name = "type_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this task type"),
        Field(
            name = "name",
            type = aftermath.types.on_disk.am_dsk_string,
            comment = "Name of this task (e.g., symbol in the executable)"),
        Field(
            name = "source",
            type = aftermath.types.on_disk.am_dsk_source_location,
            comment = "Location of the source code for this task type")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openmp_task_type,
    aftermath.types.openmp.in_memory.am_openmp_task_type)

################################################################################

am_dsk_openmp_task_instance = Frame(
    name = "am_dsk_openmp_task_instance",
    entity = "on-disk OpenMP task instance",
    comment = "An instance of an OpenMP task",

    fields = FieldList([
        Field(
            name = "type_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the task type for this instance"),
        Field(
            name = "instance_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of this task instance")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_openmp_task_instance,
    aftermath.types.openmp.in_memory.am_openmp_task_instance)

relations.join.make_join(
    dsk_src_field = am_dsk_openmp_task_instance.getFields().getFieldByName("type_id"),
    dsk_target_field = am_dsk_openmp_task_type.getFields().getFieldByName("type_id"),
    mem_ptr_field = aftermath.types.openmp.in_memory.am_openmp_task_instance.getFields().getFieldByName("task_type"),
    mem_target_type = aftermath.types.openmp.in_memory.am_openmp_task_type,
    null_allowed = False)

################################################################################

am_dsk_openmp_task_period = EventFrame(
    name = "am_dsk_openmp_task_period",
    entity = "on-disk OpenMP task execution period",
    comment = "An OpenMP task execution period (contiguous interval of " + \
              "execution of a task)",
    fields = FieldList([
        Field(
            name = "instance_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "Numerical ID of the OpenMP task instance this " + \
                      "period accounts for"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_openmp_task_period,
    aftermath.types.openmp.in_memory.am_openmp_task_period,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_openmp_task_period.getFields().getFieldByName("instance_id"),
    dsk_target_field = am_dsk_openmp_task_instance.getFields().getFieldByName("instance_id"),
    mem_ptr_field = aftermath.types.openmp.in_memory.am_openmp_task_period.getFields().getFieldByName("task_instance"),
    mem_target_type = aftermath.types.openmp.in_memory.am_openmp_task_instance,
    null_allowed = False
)

################################################################################

all_types = TypeList([
    am_dsk_openmp_for_loop_type,
    am_dsk_openmp_for_loop_instance,
    am_dsk_openmp_iteration_set,
    am_dsk_openmp_iteration_period,
    am_dsk_openmp_task_type,
    am_dsk_openmp_task_instance,
    am_dsk_openmp_task_period
])

aftermath.config.addDskTypes(*all_types)
