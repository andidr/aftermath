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

am_openstream_task_type = InMemoryCompoundType(
    name = "am_openstream_task_type",
    entity = "OpenStream task type",
    comment = "An OpenStream task type",
    ident = "am::openstream::task_type",

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

am_openstream_task_instance = InMemoryCompoundType(
    name = "am_openstream_task_instance",
    entity = "OpenStream task instance",
    comment = "An OpenStream task instance",
    ident = "am::openstream::task_instance",

    fields = FieldList([
        Field(
            name = "task_type",
            field_type = am_openstream_task_type,
            is_pointer = True,
            comment = "Type of this task instance")]))

################################################################################

am_openstream_task_period = InMemoryCompoundType(
        name = "am_openstream_task_period",
        entity = "OpenStream task execution period",
        comment = "An OpenStream task execution period",
        ident = "am::openstream::task_period",

        fields = FieldList([
            Field(
                name = "task_instance",
                field_type = am_openstream_task_instance,
                is_pointer = True,
                comment = "Task execution instance this period belongs to"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the task execution period")]))

################################################################################

all_types = TypeList([
    am_openstream_task_type,
    am_openstream_task_instance,
    am_openstream_task_period
])

aftermath.config.addMemTypes(*all_types)
