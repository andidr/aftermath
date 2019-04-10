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

from aftermath.types import TypeList, Field, FieldList, EnumType, EnumVariant
from aftermath.types.in_memory import InMemoryCompoundType
from aftermath import tags
import aftermath.types.base

# Enums
am_telamon_deadend_cause = EnumType(
    name = "enum am_telamon_deadend_cause",
    entity = "Telamon deadend cause",
    comment = "Reason for which a candidate was marked as a deadend",
    variants = [
        EnumVariant(
            "AM_TELAMON_DEADEND_CAUSE_CONSTRAINTS",
            0,
            "Candidate became a deadend because of unsatisfiable constraints"),

        EnumVariant(
            "AM_TELAMON_DEADEND_CAUSE_MODEL_CUT",
            1,
            "Candidate became a deadend because the bound was below the current"+
            "value of the cut"),

        EnumVariant(
            "AM_TELAMON_DEADEND_CAUSE_BACKTRACK",
            2,
            "Candidate became a deadend because all of its children have been" +
            "identified as deadends")
    ])

am_telamon_child_selector_type = EnumType(
    name = "enum am_telamon_child_selector_type",
    entity = "Telamon child selector type",
    comment = "Type of the child selector used for a select child action",
    variants = [
        EnumVariant(
            "AM_TELAMON_CHILD_SELECTOR_RANDOM",
            0,
            "Selector selecting a random edge"),

        EnumVariant(
            "AM_TELAMON_CHILD_SELECTOR_MAXIMUM",
            1,
            "Selector selecting the edge with the maximum value"),

        EnumVariant(
            "AM_TELAMON_CHILD_SELECTOR_EXACT",
            2,
            "Selector selecting exactly one edge")
    ])

################################################################################

am_telamon_candidate = InMemoryCompoundType(
    name = "am_telamon_candidate",
    entity = "Telamon candidate",
    comment = "A Telamon candidate",
    ident = "am::telamon::candidate",

    tags = [
        aftermath.tags.Destructor(),
        tags.mem.dfg.DeclareConstPointerType()
    ],

    fields = FieldList([
        Field(
            name = "id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of this candidate"),
        Field(
            name = "discovery_time",
            field_type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp when the candidate was discovered"),
        Field(
            name = "internal_time",
            field_type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp when the candidate was declared an internal " +
            "node (if applicable)"),
        Field(
            name = "rollout_time",
            field_type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp when the candidate was first encountered " +
            "as a rollout node (if applicable)"),
        Field(
            name = "implementation_time",
            field_type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp when the candidate was marked as an " +
            "implementation (if applicable)"),
        Field(
            name = "deadend_time",
            field_type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp when the candidate was first identified as " +
            "a deadend (if applicable)"),
        Field(
            name = "flags",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Flags (e.g., deadend, etc.)"),
        Field(
            name = "perfmodel_bound",
            field_type = aftermath.types.builtin.double,
            comment = "Lower bound as calculated by the performance model"),
        Field(
            name = "score",
            field_type = aftermath.types.builtin.double,
            comment = "Score from evaluation"),
        Field(
            name = "action",
            field_type = aftermath.types.base.am_string,
            comment = "Action for this candidate wrt its parent")]))

am_telamon_candidate.getFields().prependFields([
    Field(
        name = "parent",
        field_type = am_telamon_candidate,
        is_pointer = True,
        is_owned = False,
        comment = "Parent candidate"),
    Field(
        name = "num_children",
        field_type = aftermath.types.builtin.size_t,
        comment = "Number of child candidates"),
    Field(
        name = "children",
        field_type = am_telamon_candidate,
        is_pointer = True,
        is_owned = False,
        is_array = True,
        array_num_elements_field_name = "num_children",
        pointer_depth = 2,
        comment = "Parent candidate")])

am_telamon_candidate.addTag(
    aftermath.tags.GenerateDefaultConstructor(field_values = [
        ("parent", "NULL"),
        ("num_children", "0"),
        ("children", "NULL")
    ]))

################################################################################

am_telamon_thread_trace = InMemoryCompoundType(
    name = "am_telamon_thread_trace",
    entity = "Telamon thread trace",
    comment = "Beginning and end of a series of actions performed by a thread",
    ident = "am::telamon::thread_trace",

    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
            comment = "Interval fully including all actions of the thread trace"
        )
    ]))

################################################################################

am_telamon_candidate_evaluate_action = InMemoryCompoundType(
    name = "am_telamon_candidate_evaluate_action",
    entity = "Telamon candidate evaluate action",
    comment = "Attempt to evaluate a Telamon candidate",
    ident = "am::telamon::action::evaluate_candidate",

    fields = FieldList([
        Field(
            name = "candidate",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Candidate that was evaluated"),
        Field(
            name = "timestamp",
            field_type = aftermath.types.base.am_timestamp_t,
            comment = "Timestamp at which the result of the evaluation was " +
            "backpropagated"),
        Field(
            name = "flags",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Flags (e.g., score valid)"),
        Field(
            name = "score",
            field_type = aftermath.types.builtin.double,
            comment = "Score from the evaluation (if valid)"),
    ]))

################################################################################

am_telamon_candidate_expand_action = InMemoryCompoundType(
    name = "am_telamon_candidate_expand_action",
    entity = "Telamon candidate expand action",
    comment = "Expandion of a Telamon candidate during search",
    ident = "am::telamon::action::expand_candidate",

    fields = FieldList([
        Field(
            name = "candidate",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Candidate that was expanded"),
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
            comment = "Interval during which the candidate was expanded"
        )
    ]))

################################################################################

am_telamon_candidate_kill_action = InMemoryCompoundType(
    name = "am_telamon_candidate_kill_action",
    entity = "Telamon candidate kill action",
    comment = "Action carried out on a Telamon candidate marking it as a deadend",
    ident = "am::telamon::action::kill_candidate",

    fields = FieldList([
        Field(
            name = "candidate",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Candidate that was marked as a deadend"),
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
            comment = "Interval during which the candidate was marked as a deadend"
        ),
        Field(
            name = "cause",
            field_type = am_telamon_deadend_cause,
            comment = "Cause for which the node was marked as a deadend")
    ]))

################################################################################

am_telamon_candidate_mark_implementation_action = InMemoryCompoundType(
    name = "am_telamon_candidate_mark_implementation_action",
    entity = "Telamon candidate mark implementation action",
    comment = "Marks a Telamon candidate as an implementation",
    ident = "am::telamon::action::mark_implementation",

    fields = FieldList([
        Field(
            name = "candidate",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Candidate that was marked as an implementation"),
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
            comment = "Interval during which the candidate was marked"
        )
    ]))

################################################################################

am_telamon_candidate_select_action = InMemoryCompoundType(
    name = "am_telamon_candidate_select_action",
    entity = "Telamon candidate select action",
    comment = "Selection of a Telamon candidate during search",
    ident = "am::telamon::action::select_candidate",

    fields = FieldList([
        Field(
            name = "candidate",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Candidate that was selected"),
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
            comment = "Interval during which the candidate was selected"
        )
    ]))

################################################################################

am_telamon_child_selector_entry = InMemoryCompoundType(
    name = "am_telamon_child_selector_entry",
    entity = "Telamon child selector entry",
    comment = "Telamon child selector entry",
    fields = FieldList([
        Field(
            name = "edge_idx",
            field_type = aftermath.types.builtin.uint16_t,
            comment = "Index of the child edge"),
        Field(
            name = "value",
            field_type = aftermath.types.builtin.double,
            comment = "Selector-dependent value associated to the child edge")
    ]))

am_telamon_child_selector = InMemoryCompoundType(
    name = "am_telamon_child_selector",
    entity = "Telamon child selector",
    comment = "Telamon child selector",
    fields = FieldList([
        Field(
            name = "type",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Numerical ID for the type"),
        Field(
            name = "num_entries",
            field_type = aftermath.types.builtin.uint16_t,
            comment = "Number of entries of the vector with child selection data"),
        Field(
            name = "entries",
            field_type = am_telamon_child_selector_entry,
            is_pointer = True,
            is_owned = True,
            is_array = True,
            array_num_elements_field_name = "num_entries",
            comment = "Actual entries")
    ]))

################################################################################

am_telamon_candidate_select_child_action = InMemoryCompoundType(
    name = "am_telamon_candidate_select_child_action",
    entity = "Telamon candidate select child action",
    comment = "Selection of the child of a Telamon candidate during search",
    ident = "am::telamon::action::select_child",

    fields = FieldList([
        Field(
            name = "candidate",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Candidate whose child was selected"),
        Field(
            name = "child",
            field_type = am_telamon_candidate,
            is_pointer = True,
            is_owned = False,
            comment = "Child that was selected"),
        Field(
            name = "child_idx",
            field_type = aftermath.types.builtin.uint16_t,
            comment = "Index of the selected child"),
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
            comment = "Interval during which the candidate was selected"
        ),
        Field(
            name = "selector",
            field_type = am_telamon_child_selector,
            comment = "Selector that selected the child"
        )
    ]))

################################################################################

all_types = TypeList([
    am_telamon_deadend_cause,
    am_telamon_child_selector_type,
    am_telamon_candidate,
    am_telamon_thread_trace,
    am_telamon_candidate_evaluate_action,
    am_telamon_candidate_expand_action,
    am_telamon_candidate_kill_action,
    am_telamon_candidate_mark_implementation_action,
    am_telamon_candidate_select_action,
    am_telamon_child_selector,
    am_telamon_child_selector_entry,
    am_telamon_candidate_select_child_action
])

aftermath.config.addMemTypes(*all_types)
