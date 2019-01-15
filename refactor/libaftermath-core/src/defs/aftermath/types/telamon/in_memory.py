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
            name = "score",
            type = aftermath.types.builtin.double,
            comment = "Score")]))

am_telamon_candidate.getFields().prependFields([
    Field(
        name = "parent",
        type = am_telamon_candidate,
        is_pointer = True,
        is_owned = False,
        comment = "Parent candidate"),
    Field(
        name = "num_children",
        type = aftermath.types.builtin.size_t,
        comment = "Number of child candidates"),
    Field(
        name = "children",
        type = am_telamon_candidate,
        is_pointer = True,
        is_owned = False,
        pointer_depth = 2,
        comment = "Parent candidate")])

am_telamon_candidate.addTag(
    aftermath.tags.GenerateDefaultConstructor(field_values = [
        ("parent", "NULL"),
        ("num_children", "0"),
        ("children", "NULL")
    ]))

################################################################################

all_types = TypeList([
    am_telamon_candidate
])

aftermath.config.addMemTypes(*all_types)
