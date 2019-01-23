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

from aftermath.relations.collect import make_collect_pointers, SelfMode
from aftermath.relations.join import make_join
from aftermath.tags.postprocess.graph import \
    GenerateCollectGraphRootsFunction, \
    GenerateCheckRootCountFunction
from aftermath.tags.finalize import GenerateFinalizeFunction
from aftermath.tags import FunctionTagStep

def make_tree_relation(
        dsk_parent_id_field,
        dsk_id_field,
        mem_parent_field,
        mem_num_children_field,
        mem_children_field,
        rootp_array_struct_name,
        rootp_array_ident):
    """Builds a tree relation, from an on id - parent_id relationships in on-disk
    data structures to a parent pointer and array of pointers for children in
    in-memory structures. A pointer to the root of a tree is added to an array
    of roots.

    `dsk_parent_id_field` is the field of the on-disk structure containing the
    id of its parent.

    `dsk_id_field` is the field of the on-disk structure containing the id of
    the node itself.

    `mem_parent_field` is a pointer field from a node to its parent.

    `mem_num_children_field` is a field that counts the number of children of a
    node.

    `mem_children_field` is a field that stores a pointer to an array of node
    pointers representing the node's children.

    `rootp_array_struct_name` is the name of the type of the array to which the
    pointer to the tree's root is added.

    `rootp_array_ident` is the string identifying the per-trace array to which
    the root is added.
    """

    if dsk_parent_id_field.getCompoundType() != dsk_id_field.getCompoundType():
       raise Exception("ID and parent ID fields must be fields of the same "+
                       "on-disk compound type")

    if mem_parent_field.getCompoundType() != mem_num_children_field.getCompoundType() or \
       mem_parent_field.getCompoundType() != mem_children_field.getCompoundType():
       raise Exception("Parent, children and num_children fields must be " +
                       "fields of the same in-memory compound type")

    mem_type = mem_parent_field.getCompoundType()

    make_join(
        dsk_src_field = dsk_parent_id_field,
        dsk_target_field = dsk_id_field,
        mem_ptr_field = mem_parent_field,
        mem_target_type = mem_type,
        null_allowed = False)

    make_collect_pointers(mem_src_ptr_field = mem_parent_field,
                          mem_target_num_field = mem_num_children_field,
                          mem_target_array_field = mem_children_field,
                          null_allowed = True,
                          self_mode = SelfMode.SETNULL)

    finfun = mem_type.getOrAddTagInheriting(GenerateFinalizeFunction)

    crf = GenerateCollectGraphRootsFunction(
        parent_field = mem_parent_field,
        target_array_struct_name = rootp_array_struct_name,
        target_array_ident = rootp_array_ident)
    mem_type.addTag(crf)
    finfun.addStep(FunctionTagStep(crf))

    crc = GenerateCheckRootCountFunction(
        node_array_struct_name = mem_type.getName()+"_array",
        node_array_ident = mem_type.getIdent(),
        rootp_array_struct_name = rootp_array_struct_name,
        rootp_array_ident = rootp_array_ident,
        min_count = 1,
        max_count = 1,
        triggers_only_if_at_leats_one_node = True)
    mem_type.addTag(crc)
    finfun.addStep(FunctionTagStep(crc))

