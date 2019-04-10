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

am_openmp_for_loop_type = InMemoryCompoundType(
    name = "am_openmp_for_loop_type",
    entity = "OpenMP for loop type",
    comment = "An OpenMP for loop type",
    ident = "am::openmp::for_loop_type",

    fields = FieldList([
        Field(
            name = "source",
            field_type = aftermath.types.in_memory.am_source_location,
            comment = "Location of the source code for this for loop type")]))

################################################################################

am_openmp_for_loop_instance = InMemoryCompoundType(
    name = "am_openmp_for_loop_instance",
    entity = "OpenMP for loop instance",
    comment = "An OpenMP for loop instance",
    ident = "am::openmp::for_loop_instance",

    fields = FieldList([
        Field(
            name = "loop_type",
            field_type = am_openmp_for_loop_type,
            is_pointer = True,
            comment = "Type of this for loop instance"),
        Field(
            name = "lower_bound",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "First value (included) for this loop instance"),
        Field(
            name = "upper_bound",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Last value (included) for this loop instance"),
        Field(
            name = "num_workers",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Number of workers that participated in the execution of "\
            "this loop instance")]))

################################################################################

am_openmp_iteration_set = InMemoryCompoundType(
    name = "am_openmp_iteration_set",
    entity = "OpenMP for loop iteration_set",
    comment = "A set of iterations executed by a worker",
    ident = "am::openmp::iteration_set",

    fields = FieldList([
        Field(
            name = "loop_instance",
            field_type = am_openmp_for_loop_instance,
            is_pointer = True,
            comment = "Loop instance this iteration set belongs to"),
        Field(
            name = "lower_bound",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "First value (included) for this iteration set"),
        Field(
            name = "upper_bound",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Last value (included) for this iteration set")]))

################################################################################

am_openmp_iteration_period = InMemoryCompoundType(
    name = "am_openmp_iteration_period",
    entity = "OpenMP iteration execution period",
    comment = "An OpenMP iteration execution period",
    ident = "am::openmp::iteration_period",

        fields = FieldList([
            Field(
                name = "iteration_set",
                field_type = am_openmp_iteration_set,
                is_pointer = True,
                comment = "Iteration set this period belongs to"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_openmp_task_type = InMemoryCompoundType(
    name = "am_openmp_task_type",
    entity = "OpenMP task type",
    comment = "An OpenMP task type",
    ident = "am::openmp::task_type",

    fields = FieldList([
        Field(
            name = "name",
            field_type = aftermath.types.base.am_string,
            comment = "Name of this task (e.g., symbol in the executable)"),
        Field(
            name = "source",
            field_type = aftermath.types.in_memory.am_source_location,
            comment = "Location of the source code for this task type")]))

################################################################################

am_openmp_task_instance = InMemoryCompoundType(
    name = "am_openmp_task_instance",
    entity = "OpenMP task instance",
    comment = "An OpenMP task instance",
    ident = "am::openmp::task_instance",

    fields = FieldList([
        Field(
            name = "task_type",
            field_type = am_openmp_task_type,
            is_pointer = True,
            comment = "Type of this task instance")]))

################################################################################

am_openmp_task_period = InMemoryCompoundType(
    name = "am_openmp_task_period",
    entity = "OpenMP task execution period",
    comment = "An OpenMP task execution period",
    ident = "am::openmp::task_period",

        fields = FieldList([
            Field(
                name = "task_instance",
                field_type = am_openmp_task_instance,
                is_pointer = True,
                comment = "Task execution instance this period belongs to"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the task execution period")]))

################################################################################

all_types = TypeList([
    am_openmp_for_loop_type,
    am_openmp_for_loop_instance,
    am_openmp_iteration_set,
    am_openmp_iteration_period,
    am_openmp_task_type,
    am_openmp_task_instance,
    am_openmp_task_period
])

aftermath.config.addMemTypes(*all_types)
