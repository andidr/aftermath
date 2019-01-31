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
            name = "exploration_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was first encountered " +
            "as an exploration node (if applicable)"),
        Field(
            name = "rollout_time",
            type = aftermath.types.builtin.uint64_t,
            comment = "Timestamp when the candidate was first encountered " +
            "as a rollout node (if applicable)"),
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

am_dsk_telamon_evaluation = EventFrame(
    name = "am_dsk_telamon_evaluation",
    entity = "on-disk Telamon evaluation",
    comment = "A Telamon Evaluation",
    fields = FieldList([
        Field(
            name = "candidate_id",
            type = aftermath.types.builtin.uint64_t,
            comment = "ID of the candidate that was evaluated"),
        Field(
            name = "interval",
            type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Interval during which the evaluation took place")
    ]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_telamon_evaluation,
    aftermath.types.telamon.in_memory.am_telamon_evaluation,
    "collection_id")

################################################################################

all_types = TypeList([
    am_dsk_telamon_candidate,
    am_dsk_telamon_evaluation
])

aftermath.config.addDskTypes(*all_types)
