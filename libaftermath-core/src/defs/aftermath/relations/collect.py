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

from aftermath.types import Field, FieldList, Type, CompoundType
from aftermath.tags import FunctionTagStep
from aftermath.tags.finalize import GenerateFinalizeFunction
from aftermath.util import enforce_type
import aftermath

from aftermath.tags.mem.collect import \
    CollectTarget, \
    CollectTargets, \
    CollectSource, \
    CollectSources, \
    GenerateProcessCollectSourcesFunction, \
    SelfMode, \
    SelfModeVal

def make_collect_pointers(mem_src_ptr_field,
                          mem_target_num_field,
                          mem_target_array_field,
                          null_allowed,
                          self_mode = SelfMode.ALLOWED):
    """Adds all tags necessary to the types that the specified fields belong to in
    order to implement a relation, where for each instance of the structure to
    which `mem_src_ptr_field` belongs, a pointer to the instance is added to the
    array `mem_target_array_field` of the targeted structure. The total number
    of array members will be stored in the targeted structures field
    `mem_target_num_field`.

    +-------------+
    | Struct A    |
    |             |--- mem_src_ptr_field ---->+--------------+
    |             |                        +->| Struct B     |
    +-------------+                        |  |              |
                                           |  |              |
    +-------------+                        |  +--------------+
    | Struct A    |                        |
    |             |--- mem_src_ptr_field --+
    |             |
    +-------------+

    Becomes:

    +->+-------------+
    |  | Struct A    |
    |  |             |--- mem_src_ptr_field ---->+-------------------------+
    |  |             |                        +->| Struct B                |
    |  +-------------+                        |  | mem_target_num_field: 2 | ---+
    |                                         |  |                         |    |
    |+>+-------------+                        |  +-------------------------+    |
    || | Struct A    |                        |                                 |
    || |             |--- mem_src_ptr_field --+                                 |
    || |             |                                                          |
    || +-------------+                                 mem_target_array_field -+
    ||                                                 [**]
    ||                                                  ||
    |+--------------------------------------------------+|
    +----------------------------------------------------+
    """

    enforce_type(mem_src_ptr_field, Field)
    enforce_type(mem_target_num_field, Field)
    enforce_type(mem_target_array_field, Field)
    enforce_type(null_allowed, bool)
    enforce_type(self_mode, SelfModeVal)

    mem_target_array_field.setCustomDestructorName("free");

    if not mem_src_ptr_field.getType() == mem_target_array_field.getType():
        raise Exception("Source field is of type " +
                        "'" + mem_src_field.getType().getName() + "', "
                        "but target array collects pointers to "+
                        "'" + mem_target_array_field.getType().getName() + "'")

    if mem_target_array_field.getCompoundType() != \
       mem_target_num_field.getCompoundType():
        raise Exception("Field for array and element count must be in the "+
                        "same structure; However, the array is from " +
                        "'" + mem_target_array_field.getCompoundType().getName() + "', "
                        "and the count field is from "+
                        "'" + mem_target_num_field.getCompoundType().getName() + "'")

    mem_source_t = mem_src_ptr_field.getCompoundType()
    mem_target_t = mem_target_array_field.getCompoundType()

    # Collect source
    collect_sources = mem_source_t.getOrAddTagInheriting(CollectSources)
    cs = CollectSource(mem_src_ptr_field, null_allowed, self_mode)

    if not collect_sources.hasSource(cs):
        collect_sources.addSource(cs)

    # Collect target
    collect_targets = mem_target_t.getOrAddTagInheriting(CollectTargets)
    ct = CollectTarget(mem_target_num_field, mem_target_array_field)

    if not collect_targets.hasTarget(ct):
        collect_targets.addTarget(ct)

    cs.setAssociatedTarget(ct)
    ct.setAssociatedSource(cs)

    (gen_tag, create) = mem_source_t.getOrAddTagInheritingRes(
        GenerateProcessCollectSourcesFunction)

    if create:
        source_fin = mem_source_t.getOrAddTagInheriting(GenerateFinalizeFunction)
        source_fin.addStep(FunctionTagStep(gen_tag))
