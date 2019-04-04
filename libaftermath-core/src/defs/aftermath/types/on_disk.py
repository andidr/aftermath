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

from aftermath.types import TypeList, CompoundType, Field, FieldList
from aftermath import tags
from aftermath.types import relations

import aftermath.types.builtin
import aftermath.types.in_memory

#################################################################################

class OnDiskCompoundType(CompoundType):
    """An on-disk structure"""

    def __init__(self, *args, **kwargs):
        super(OnDiskCompoundType, self).__init__(*args, **kwargs)
        self.addTags(
            tags.Packed(),
            tags.dsk.GenerateReadFunction(),
            tags.dsk.GenerateWriteFunction(),
            tags.dsk.GenerateWriteToBufferFunction(),
            tags.dsk.GenerateDumpStdoutFunction())

#################################################################################

class Frame(OnDiskCompoundType):
    """An on-disk frame (preceded by a type field)"""

    def __init__(self, *args, **kwargs):
        super(Frame, self).__init__(*args, **kwargs)

        self.addTags(
            tags.dsk.Frame(),
            tags.dsk.GenerateLoadFunction(),
            tags.dsk.GenerateWriteToBufferWithDefaultIDFunction(),
            tags.dsk.GenerateWriteDefaultIDToBufferFunction(),
            tags.dsk.GenerateWriteWithDefaultIDFunction(),
            tags.dsk.GenerateWriteDefaultIDFunction())

        # A frame is always preceded by an integer indicating its type
        self.getTagInheriting(tags.dsk.WriteFunction).setTypeParam(True)
        self.getTagInheriting(tags.dsk.WriteToBufferFunction).setTypeParam(True)

#################################################################################

class EventFrame(Frame):
    """A frame that defines an event that belongs to an event collection"""

    def __init__(self, *args, **kwargs):
        super(EventFrame, self).__init__(*args, **kwargs)

        # Add field for type
        self.addField(Field(
            name = "collection_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "ID of the event collection this event belongs to"),
        0)

#################################################################################

am_dsk_interval = OnDiskCompoundType(
    name = "am_dsk_interval",
    entity = "on-disk interval",
    comment = "An interval with a start and end timestamp",
    fields = FieldList([
        Field(
            name = "start",
            type = aftermath.types.builtin.uint64_t,
            comment = "Start of the interval"),
        Field(
            name = "end",
            type = aftermath.types.builtin.uint64_t,
            comment = "End of the interval")]))

am_dsk_interval.addTags(
    tags.assertion.GenerateAssertFunction(steps = [
        tags.assertion.AssertStep("{{start}} <= {{end}}",
                                  "Interval cannot end before it starts")
    ]),

    tags.dsk.tomem.GenerateConversionFunction(
        am_dsk_interval,
        aftermath.types.in_memory.am_interval))

#################################################################################

am_dsk_string = OnDiskCompoundType(
    name = "am_dsk_string",
    entity = "on-disk string",
    comment = "A string",
    fields = FieldList([
        Field(
            name = "len",
            type = aftermath.types.builtin.uint32_t,
            comment = "Number of bytes for the string"),
        Field(
            name = "str",
            type = aftermath.types.builtin.charp,
            comment = "Characters of the string (not zero-terminated)")]))

# Custom conversion function, so just make the name available
am_dsk_string.addTags(
    tags.dsk.tomem.ConversionFunction(
        am_dsk_string,
        aftermath.types.base.am_string,
        fieldmap = []),

    tags.dsk.ReadFunction(),
    tags.dsk.WriteFunction(),
    tags.dsk.WriteToBufferFunction(),
    tags.dsk.DumpStdoutFunction())

am_dsk_string.removeTags(
    tags.dsk.GenerateDumpStdoutFunction,
    tags.dsk.GenerateReadFunction,
    tags.dsk.GenerateWriteFunction,
    tags.dsk.GenerateWriteToBufferFunction,
    tags.Packed,
)

#################################################################################

am_dsk_source_location = OnDiskCompoundType(
    name = "am_dsk_source_location",
    entity = "on-disk source location",
    comment = 'A source location (filename, line, character).',
    fields = FieldList([
        Field(
            name = "file",
            type = am_dsk_string,
            comment = "File containing the source"),
        Field(
            name = "line",
            type = aftermath.types.builtin.uint64_t,
            comment = "Zero-indexed number of the line within the file"),
        Field(
            name = "character",
            type = aftermath.types.builtin.uint64_t,
            comment = "Zero-indexed number of the character within the line")]))

am_dsk_source_location.addTags(
    tags.dsk.tomem.GenerateConversionFunction(
        am_dsk_source_location,
        aftermath.types.in_memory.am_source_location))

#################################################################################

am_dsk_header = OnDiskCompoundType(
    name = "am_dsk_header",
    entity = "Trace file header",
    comment = "Header identifying Aftermath trace files",

    fields = FieldList([
        Field(name = "magic",
              type = aftermath.types.builtin.uint32_t,
              comment = "Magic number for Aftermath trace files"),
        Field(name = "version",
              type = aftermath.types.builtin.uint32_t,
              comment = "Version of the trace format")]))

#################################################################################

am_dsk_counter_event = EventFrame(
    name = "am_dsk_counter_event",
    entity = "on-disk counter sample",
    comment = "A counter event",
    fields = FieldList([
        Field(
            name = "counter_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the counter this sample is for"),
        Field(
            name = "time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp of the sample"),
        Field(
            name = "value",
            type = aftermath.types.builtin.int64_t,
            comment = "Value of the counter sample")]))

tags.dsk.tomem.add_per_event_collection_sub_array_tags(
    am_dsk_counter_event,
    aftermath.types.in_memory.am_counter_event,
    "collection_id",
    "counter_id",
    event_collection_array_struct_name = "am_counter_event_array_collection")

#################################################################################

am_dsk_measurement_interval = Frame(
    name = "am_dsk_measurement_interval",
    entity = "on-disk measurement interval",
    comment = "A Measurement interval marking an interval in the trace",
    fields = FieldList([
        Field(
            name = "interval",
            type =  am_dsk_interval,
            comment = "Start and end of the measurement interval")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_measurement_interval,
    aftermath.types.in_memory.am_measurement_interval)

#################################################################################

am_dsk_hierarchy_node = Frame(
    name = "am_dsk_hierarchy_node",
    entity = "on-disk hierarchy node",
    comment = "A node in a hierarchy",
    fields = FieldList([
        Field(
            name = "hierarchy_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the hierarchy this node belongs to"),
        Field(
            name = "id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of this hierarchy node"),
        Field(
            name = "parent_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the parent of this node"),
        Field(
            name = "name",
            type = am_dsk_string,
            comment = "Name of this node")]))

am_dsk_hierarchy_node.addTag(tags.process.ProcessFunction())

#################################################################################

am_dsk_frame_type_id = Frame(
    name = "am_dsk_frame_type_id",
    entity = "frame type ID association",
    comment = "Association of a frame type with a frame ID",
    fields = FieldList([
        Field(
            name = "id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID that will be associated with the type"),
        Field(
            name = "type_name",
            type = am_dsk_string,
            comment = "Frame type as a string")]))

am_dsk_frame_type_id.addTag(tags.process.ProcessFunction())
am_dsk_frame_type_id.getTagInheriting(tags.dsk.WriteToBufferFunction).setTypeParam(False)
am_dsk_frame_type_id.getTagInheriting(tags.dsk.WriteToBufferFunction).setConstantTypeID(0)

#################################################################################

am_dsk_event_collection = Frame(
    name = "am_dsk_event_collection",
    entity = "on-disk event collection",
    comment = "Definition of an event collection",
    fields = FieldList([
        Field(
            name = "id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of this event collection"),
        Field(
            name = "name",
            type = am_dsk_string,
            comment = "Name of this event collection")]))

am_dsk_event_collection.addTag(tags.process.ProcessFunction())

#################################################################################

am_dsk_state_description = Frame(
    name = "am_dsk_state_description",
    entity = "on-disk state description",
    comment = "Frame providing a human-readable description of a state",

    fields = FieldList([
        Field(
            name = "state_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the state"),
        Field(
            name = "name",
            type = am_dsk_string,
            comment = "Name of the state")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_state_description,
    aftermath.types.in_memory.am_state_description)

################################################################################

am_dsk_state_event = EventFrame(
    name = "am_dsk_state_event",
    entity = "on-disk state event",
    comment = "A state event",

    fields = FieldList([
        Field(
            name = "state",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the state"),
        Field(
            name = "interval",
            type = am_dsk_interval,
            comment = "Interval during which the state was active")]))

conversion_fun_tag = tags.dsk.tomem.GenerateConversionFunction(
        am_dsk_state_event,
        aftermath.types.in_memory.am_state_event,
        fieldmap = [ ("state", "state_idx"), "interval" ])

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_state_event,
    aftermath.types.in_memory.am_state_event,
    "collection_id",
    conversion_fun_tag = conversion_fun_tag)

am_dsk_state_event.addTags(tags.assertion.AssertFunction())

relations.join.make_join(
    dsk_src_field = am_dsk_state_event.getFields().getFieldByName("state"),
    dsk_target_field = am_dsk_state_description.getFields().getFieldByName("state_id"),
    mem_ptr_field = aftermath.types.in_memory.am_state_event.getFields().getFieldByName("state"),
    mem_target_type = aftermath.types.in_memory.am_state_description,
    null_allowed = False)

################################################################################

am_dsk_counter_description = Frame(
    name = "am_dsk_counter_description",
    entity = "on-disk counter description",
    comment = "A human-readable description of a counter",
    fields = FieldList([
        Field(
            name = "counter_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the counter"),
        Field(
            name = "name",
            type = am_dsk_string,
            comment = "Name of the counter")]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_counter_description,
    aftermath.types.in_memory.am_counter_description)

################################################################################

am_dsk_event_mapping = EventFrame(
    name = "am_dsk_event_mapping",
    entity = "on-disk event mapping",
    comment = "A frame associating an event collection to a hierarchy node " + \
              "for an interval",
    fields = FieldList([
        Field(
            name = "hierarchy_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the hierarchy for the mapping"),
        Field(
            name = "node_id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of the hierarchy node for the mapping"),
        Field(
            name = "interval",
            type = am_dsk_interval,
            comment = "Interval during which the event collection was " + \
            "associated to the node")]))

am_dsk_event_mapping.addTag(tags.process.ProcessFunction())

################################################################################

am_dsk_hierarchy_description = Frame(
    name = "am_dsk_hierarchy_description",
    entity = "on-disk hierarchy description",
    comment = "Human-readable description of a hierarchy",

    fields = FieldList([
        Field(
            name = "id",
            type = aftermath.types.builtin.uint32_t,
            comment = "Numerical ID of this hierarchy"),
        Field(
            name = "name",
            type = am_dsk_string,
            comment = "Name of the hierarchy")]))

am_dsk_hierarchy_description.addTag(tags.process.ProcessFunction())

################################################################################

all_types = TypeList([
    am_dsk_interval,
    am_dsk_string,
    am_dsk_source_location,
    am_dsk_header,
    am_dsk_counter_event,
    am_dsk_measurement_interval,
    am_dsk_hierarchy_node,
    am_dsk_frame_type_id,
    am_dsk_event_collection,
    am_dsk_state_description,
    am_dsk_state_event,
    am_dsk_counter_description,
    am_dsk_event_mapping,
    am_dsk_hierarchy_description
])

aftermath.config.addDskTypes(*all_types)
