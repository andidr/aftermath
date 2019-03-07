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
from aftermath.types.on_disk import Frame, EventFrame, OnDiskCompoundType
import aftermath.types.on_disk

am_dsk_telamon_candidate = Frame(
    name = "am_dsk_telamon_candidate",
    entity = "on-disk Telamon candidate",
    comment = "A Telamon Candidate",
    fields = FieldList([
        Field(
            name = "id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of this candidate"),
        Field(
            name = "parent_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the parent of this candidate"),
        Field(
            name = "discovery_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was discovered "),
        Field(
            name = "internal_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was declared an internal " +
            "node (if applicable)"),
        Field(
            name = "rollout_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was first encountered " +
            "as a rollout node (if applicable)"),
        Field(
            name = "implementation_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was marked as an " +
            "implementation (if applicable)"),
        Field(
            name = "deadend_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was first identified as " +
            "a deadend (if applicable)"),
        Field(
            name = "flags",
            type = aftermath.types.builtin.uint32_t,
            comment = "Flags (e.g., deadend, etc.)"),
        Field(
            name = "perfmodel_bound",
            type = aftermath.types.builtin.double,
            comment = "Lower bound as calculated by the performance model"),
        Field(
            name = "score",
            type = aftermath.types.builtin.double,
            comment = "Score from evaluation"),
        Field(
            name = "action",
            type = aftermath.types.on_disk.am_dsk_string,
            comment = "Action for this candidate wrt its parent")
    ]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_telamon_candidate,
    aftermath.types.telamon.in_memory.am_telamon_candidate)

relations.graph.make_tree_relation(
    dsk_parent_id_field = am_dsk_telamon_candidate.getFields().getFieldByName("parent_id"),
    dsk_id_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_parent_field = aftermath.types.telamon.in_memory.am_telamon_candidate.getFields().getFieldByName("parent"),
    mem_num_children_field = aftermath.types.telamon.in_memory.am_telamon_candidate.getFields().getFieldByName("num_children"),
    mem_children_field = aftermath.types.telamon.in_memory.am_telamon_candidate.getFields().getFieldByName("children"),
    rootp_array_struct_name = "am_telamon_candidatep_array",
    rootp_array_ident = "am::telamon::candidate_root")

################################################################################

am_dsk_telamon_thread_trace = EventFrame(
    name = "am_dsk_telamon_thread_trace",
    entity = "Telamon thread trace",
    comment = "Beginning and end of a series of actions performed by a thread",
    fields = FieldList([
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval fully including all actions of the thread trace")
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_thread_trace,
    aftermath.types.telamon.in_memory.am_telamon_thread_trace,
    "collection_id")

################################################################################

am_dsk_telamon_candidate_evaluate_action = Frame(
    name = "am_dsk_telamon_candidate_evaluate_action",
    entity = "Telamon candidate evaluate action",
    comment = "Attempt to evaluate a Telamon candidate",
    fields = FieldList([
        Field(
            name = "candidate_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate that was evaluateed"),
        Field(
            name = "timestamp",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp at which the result of the evaluation was " +
            "backpropagated"),
        Field(
            name = "score",
            type = aftermath.types.builtin.double,
            comment = "Score from the evaluation (if valid)"),
        Field(
            name = "flags",
            type = aftermath.types.builtin.uint8_t,
            comment = "Flags (e.g., score valid)")
    ]))

tags.dsk.tomem.add_per_trace_array_tags(
    am_dsk_telamon_candidate_evaluate_action,
    aftermath.types.telamon.in_memory.am_telamon_candidate_evaluate_action)

relations.join.make_join(
    dsk_src_field = am_dsk_telamon_candidate_evaluate_action.getFields().getFieldByName("candidate_id"),
    dsk_target_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.telamon.in_memory.am_telamon_candidate_evaluate_action.getFields().getFieldByName("candidate"),
    mem_target_type = aftermath.types.telamon.in_memory.am_telamon_candidate,
    null_allowed = False)

################################################################################

am_dsk_telamon_candidate_expand_action = EventFrame(
    name = "am_dsk_telamon_candidate_expand_action",
    entity = "Telamon candidate expand action",
    comment = "Expansion of a Telamon candidate during search",
    fields = FieldList([
        Field(
            name = "candidate_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate that was expanded"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval during which the action took place")
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_candidate_expand_action,
    aftermath.types.telamon.in_memory.am_telamon_candidate_expand_action,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_telamon_candidate_expand_action.getFields().getFieldByName("candidate_id"),
    dsk_target_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.telamon.in_memory.am_telamon_candidate_expand_action.getFields().getFieldByName("candidate"),
    mem_target_type = aftermath.types.telamon.in_memory.am_telamon_candidate,
    null_allowed = False)

################################################################################

am_dsk_telamon_candidate_kill_action = EventFrame(
    name = "am_dsk_telamon_candidate_kill_action",
    entity = "Telamon candidate kill action",
    comment = "Action carried out on a Telamon candidate marking it as a deadend",
    fields = FieldList([
        Field(
            name = "candidate_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate that was evaluated"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval during which the action took place"),
        Field(
            name = "cause",
            type = aftermath.types.builtin.uint8_t,
            comment = "Cause for which the node was marked as a deadend")
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_candidate_kill_action,
    aftermath.types.telamon.in_memory.am_telamon_candidate_kill_action,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_telamon_candidate_kill_action.getFields().getFieldByName("candidate_id"),
    dsk_target_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.telamon.in_memory.am_telamon_candidate_kill_action.getFields().getFieldByName("candidate"),
    mem_target_type = aftermath.types.telamon.in_memory.am_telamon_candidate,
    null_allowed = False)

################################################################################

am_dsk_telamon_candidate_mark_implementation_action = EventFrame(
    name = "am_dsk_telamon_candidate_mark_implementation_action",
    entity = "Telamon candidate mark implementation action",
    comment = "Marks a Telamon candidate as an implementation",
    fields = FieldList([
        Field(
            name = "candidate_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate that was marked as an implementation"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval during which the action took place")
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_candidate_mark_implementation_action,
    aftermath.types.telamon.in_memory.am_telamon_candidate_mark_implementation_action,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_telamon_candidate_mark_implementation_action.getFields().getFieldByName("candidate_id"),
    dsk_target_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.telamon.in_memory.am_telamon_candidate_mark_implementation_action.getFields().getFieldByName("candidate"),
    mem_target_type = aftermath.types.telamon.in_memory.am_telamon_candidate,
    null_allowed = False)

################################################################################

am_dsk_telamon_candidate_select_action = EventFrame(
    name = "am_dsk_telamon_candidate_select_action",
    entity = "Telamon candidate select action",
    comment = "Selection of a Telamon candidate during search",
    fields = FieldList([
        Field(
            name = "candidate_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate that was selected"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval during which the action took place")
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_candidate_select_action,
    aftermath.types.telamon.in_memory.am_telamon_candidate_select_action,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_telamon_candidate_select_action.getFields().getFieldByName("candidate_id"),
    dsk_target_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.telamon.in_memory.am_telamon_candidate_select_action.getFields().getFieldByName("candidate"),
    mem_target_type = aftermath.types.telamon.in_memory.am_telamon_candidate,
    null_allowed = False)

################################################################################

am_dsk_telamon_child_selector_entry = OnDiskCompoundType(
    name = "am_dsk_telamon_child_selector_entry",
    entity = "Telamon child selector entry",
    comment = "Telamon child selector entry",
    fields = FieldList([
        Field(
            name = "edge_idx",
            type = aftermath.types.builtin.uint16_t,
            comment = "Index of the child edge"),
        Field(
            name = "value",
            type = aftermath.types.builtin.double,
            comment = "Selector-dependent value associated to the child edge")
    ]))

am_dsk_telamon_child_selector_entry.addTag(
    tags.dsk.tomem.GenerateConversionFunction(
        am_dsk_telamon_child_selector_entry,
        aftermath.types.telamon.in_memory.am_telamon_child_selector_entry))

################################################################################

am_dsk_telamon_child_selector = OnDiskCompoundType(
    name = "am_dsk_telamon_child_selector",
    entity = "Telamon child selector",
    comment = "Telamon child selector",
    fields = FieldList([
        Field(
            name = "type",
            type = aftermath.types.builtin.uint8_t,
            comment = "Numerical ID for the type"),
        Field(
            name = "num_entries",
            type = aftermath.types.builtin.uint16_t,
            comment = "Number of entries of the vector with child selection data"),
        Field(
            name = "entries",
            type = am_dsk_telamon_child_selector_entry,
            is_pointer = True,
            is_owned = True,
            is_array = True,
            array_num_elements_field_name = "num_entries",
            comment = "Actual entries")
    ]))

am_dsk_telamon_child_selector.removeTags(
    tags.dsk.GenerateReadFunction,
    tags.dsk.GenerateWriteFunction,
    tags.dsk.GenerateWriteToBufferFunction,
    tags.Packed)

# Custom conversion function, so just make the name available
am_dsk_telamon_child_selector.addTags(
    tags.dsk.tomem.GenerateConversionFunction(
        am_dsk_telamon_child_selector,
        aftermath.types.telamon.in_memory.am_telamon_child_selector),

    tags.dsk.GenerateArrayReadFunction(
        num_elements_field_name = "num_entries",
        array_field_name = "entries",
        verbatim_field_names = [ "type", "num_entries" ]
    ))

################################################################################

am_dsk_telamon_candidate_select_child_action = EventFrame(
    name = "am_dsk_telamon_candidate_select_child_action",
    entity = "Telamon candidate select child action",
    comment = "Selection of the child of a Telamon candidate during search",
    fields = FieldList([
        Field(
            name = "parent_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate whose child was selected"),
        Field(
            name = "child_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the selected child"),
        Field(
            name = "child_idx",
            type = aftermath.types.builtin.uint16_t,
            comment = "Index of the selected child"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval during which the action took place"),
        Field(
            name = "selector",
            type = am_dsk_telamon_child_selector,
            comment = "Selector"),
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_candidate_select_child_action,
    aftermath.types.telamon.in_memory.am_telamon_candidate_select_child_action,
    "collection_id")

relations.join.make_join(
    dsk_src_field = am_dsk_telamon_candidate_select_child_action.getFields().getFieldByName("parent_id"),
    dsk_target_field = am_dsk_telamon_candidate.getFields().getFieldByName("id"),
    mem_ptr_field = aftermath.types.telamon.in_memory.am_telamon_candidate_select_child_action.getFields().getFieldByName("candidate"),
    mem_target_type = aftermath.types.telamon.in_memory.am_telamon_candidate,
    null_allowed = False)

am_dsk_telamon_candidate_select_child_action.removeTags(
    tags.dsk.GenerateWriteFunction,
    tags.dsk.GenerateWriteToBufferFunction,
    tags.dsk.GenerateWriteToBufferWithDefaultIDFunction,
    tags.dsk.GenerateWriteWithDefaultIDFunction,
    tags.Packed)

################################################################################

all_types = TypeList([
    am_dsk_telamon_candidate,
    am_dsk_telamon_thread_trace,
    am_dsk_telamon_candidate_evaluate_action,
    am_dsk_telamon_candidate_expand_action,
    am_dsk_telamon_candidate_kill_action,
    am_dsk_telamon_candidate_mark_implementation_action,
    am_dsk_telamon_candidate_select_action,
    am_dsk_telamon_child_selector_entry,
    am_dsk_telamon_child_selector,
    am_dsk_telamon_candidate_select_child_action
])

aftermath.config.addDskTypes(*all_types)
