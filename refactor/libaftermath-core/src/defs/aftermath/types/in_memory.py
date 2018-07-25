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

from aftermath.types import TypeList, \
    CompoundType, \
    NonCompoundType, \
    Field, \
    FieldList
import aftermath.types.base
from aftermath import tags

#################################################################################

class InMemoryCompoundType(CompoundType):
    """A CompoundType for in-memory data structures. Checks for in-memory-specific
    tags upon invocation of finalize()"""

    def __init__(self, *args, **kwargs):
        CompoundType.__init__(self, *args, **kwargs)

        # By default check if an update of the minimum and maximum timestamp for
        # the trace is needed
        self.addTag(tags.process.CheckGenerateTraceMinMaxTimestampCompoundScan())

    def finalize(self):
        CompoundType.finalize(self)

        # Check update of minimum and maximum timestamp for the trace is desired
        # and needed
        if self.hasTag(tags.process.CheckGenerateTraceMinMaxTimestampCompoundScan):
            if self.referencesType(aftermath.types.base.am_timestamp_t):
                scan_tag = tags.process.GenerateTraceMinMaxTimestampCompoundScan()
                self.addTags(scan_tag)
                process_tag = self.getOrAddTagInheriting(tags.process.GenerateProcessFunction)
                process_tag.addStep(tags.FunctionTagStep(scan_tag))

#################################################################################

am_counter_event = InMemoryCompoundType(
    name = "am_counter_event",
    entity = "counter event",
    comment = "A counter event",
    ident = "am::generic::counter_event",

    fields = FieldList([
        Field(
            name = "time",
            type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp of the counter interval"),
        Field(
            name = "value",
            type = aftermath.types.base.am_counter_value_t,
            comment = "Value of the counter event")]))

#################################################################################

am_interval = InMemoryCompoundType(
    name = "am_interval",
    entity = "interval",
    comment = "An interval with a start and end timestamp.",
    ident = "am::generic::interval",

    fields = FieldList([
        Field(
            name = "start",
            type = aftermath.types.base.am_timestamp_t,
            comment = "Start of the interval"),
        Field(
            name = "end",
            type = aftermath.types.base.am_timestamp_t,
            comment = "End of the interval")]))

#################################################################################

am_measurement_interval = InMemoryCompoundType(
    name = "am_measurement_interval",
    entity = "measurement interval",
    comment = "A measurement interval",
    ident = "am::generic::measurement_interval",

    fields = FieldList([
        Field(
            name = "interval",
            type = am_interval,
            comment = "Start and end of the measurement interval")]))

#################################################################################

am_time_offset = InMemoryCompoundType(
    name = "am_time_offset",
    entity = "timestamp difference",
    comment = "Difference between two timestamps",
    ident = "am::generic::time_offset",

    fields = FieldList([
        Field(
            name = "abs",
            type = aftermath.types.base.am_timestamp_t,
            comment = "Absolute difference"),
        Field(
            name = "sign",
            type = aftermath.types.base.am_bool_t,
            comment = "If sign != 0 the difference is negative")]))

#################################################################################

am_counter_description = InMemoryCompoundType(
    name = "am_counter_description",
    entity = "counter description",
    comment = "A description associating a name with a numerical counter ID",
    ident = "am::generic::counter_description",

    fields = FieldList([
        Field(
            name = "name",
            type = aftermath.types.base.am_string,
            comment = "Name of the counter")]))

#################################################################################

am_state_description = InMemoryCompoundType(
    name = "am_state_description",
    entity = "state description",
    comment = "A description associating a name with a numerical state ID",
    ident = "am::generic::state_description",

    fields = FieldList([
        Field(
            name = "name",
            type = aftermath.types.base.am_string,
            comment = "Name of the state")]))

#################################################################################

am_state_event = InMemoryCompoundType(
    name = "am_state_event",
    entity = "state",
    comment = 'A state (e.g., a run-time state)',
    ident = "am::generic::state",

    fields = FieldList([
        Field(
            name = "interval",
            type = am_interval,
            comment = "Interval during which the state was active"),
        Field(
            name = "state",
            type = am_state_description,
            comment = "state description",
            is_pointer = True),
        Field(
            name = "state_idx",
            type = aftermath.types.base.am_state_t,
            comment = "Index of the state"),
    ]))

#################################################################################

am_source_location = InMemoryCompoundType(
    name = "am_source_location",
    entity = "source location",
    comment = "A source code location (file, line, character)",
    ident = "am::generic::source_location",

    fields = FieldList([
        Field(
            name = "file",
            type = aftermath.types.base.am_string,
            comment = "File containing the source"),
        Field(
            name = "line",
            type = aftermath.types.base.am_source_line_t,
            comment = "Zero-indexed number of the line within the file"),
        Field(
            name = "character",
            type = aftermath.types.base.am_source_character_t,
            comment = "Zero-indexed number of the character within the line")
    ]))

#################################################################################

toplevel_types = TypeList([
    am_counter_event,
    am_measurement_interval,
    am_counter_description,
    am_state_description,
    am_state_event,
    am_source_location
])

aux_types = TypeList([
    am_interval,
    am_time_offset
])

all_types = toplevel_types + aux_types

aftermath.config.addMemTypes(*all_types)
