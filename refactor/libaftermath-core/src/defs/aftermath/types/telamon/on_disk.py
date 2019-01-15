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
            name = "score",
            type = aftermath.types.builtin.double,
            comment = "Score")
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

all_types = TypeList([
    am_dsk_telamon_candidate
])

aftermath.config.addDskTypes(*all_types)
