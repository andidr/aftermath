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
from aftermath.util import enforce_type
import aftermath

from aftermath.tags.dsk.tomem import \
    GenerateSortMetaStructArrayFunction, \
    GenerateMatchAllMetaStructArraysFunction, \
    GenerateAddToAllMetaStructArraysFunction, \
    GeneratePostConversionFunction

from aftermath.tags.postprocess import GeneratePostprocessFunction
from aftermath.tags import FunctionTagStep

from aftermath.tags.finalize import GenerateFinalizeFunction

from aftermath.tags.dsk.tomem.join import \
    DskToMetaSource, \
    DskToMetaSources, \
    JoinTarget, \
    JoinTargets, \
    JoinSource, \
    JoinSources

def make_join(dsk_src_field,
              dsk_target_field,
              mem_ptr_field,
              mem_target_type,
              null_allowed):
    """Adds all tags to the types that the specified fields belong to in order
    to implement a join relation that, such that in the end, each instance of the
    type that `mem_ptr_field` belongs to points to an instance of
    `mem_target_type`. The target instance is the instance, for which the field
    `dsk_target_field` of the associated on-disk data structure instance has the
    same value as the field `dsk_src_field` of the source's associated on-disk
    data structure instance.

    If `null_allowed` is True, there might be source data structures, for which
    no target could be found, in which case the value of `mem_ptr_field` is set
    to NULL.

    The tags that are added implement the following behavior:

      During trace loading:

        - Add a meta-data structure instance for each source data structure
          instance with the value of `dsk_src_field` into a
          per-{trace,eventcollection,evencollectionsub}array at the exact same
          index as the source data structure in its
          per-{trace,eventcollection,evencollectionsub}array.

        - Add a meta-data structure instance for each target data structure
          instance, associating the value of `dsk_target_field` with the index of
          the in-memory target data structure in its
          per-{trace,eventcollection,evencollectionsub}array.

      After trace loading (post processing):

        - Sort the arrays with target meta-data structures by the values of
          `dsk_target_field` in ascending order.

      After trace loading and after post processing (finalization step):

        - Match the source and target meta-data instances and set the pointer
          `mem_ptr_field` for each instance of the source data type to the address
          of the matched target structure.

      After trace loading, after post processing and after finalization
      (teardown step):

        - Destroy the arrays with meta-data structures
    """

    enforce_type(dsk_src_field, Field)
    enforce_type(dsk_target_field, Field)
    enforce_type(mem_ptr_field, Field)
    enforce_type(mem_target_type, Type)

    if isinstance(dsk_src_field.getType(), CompoundType):
        raise Exception("On-disk source field cannot be a compound type")

    if isinstance(dsk_target_field.getType(), CompoundType):
        raise Exception("On-disk target field cannot be a compound type")

    if dsk_src_field.getType() != dsk_target_field.getType():
        raise Exception("Join must be on on-disk fields of the same type")

    if not mem_ptr_field.isPointer():
        raise Exception("Source field of in-memory data structure must be " +
                        "a pointer")

    meta_types = aftermath.config.getMetaTypes()
    mem_source_t = mem_ptr_field.getCompoundType()
    dsk_source_t = dsk_src_field.getCompoundType()
    dsk_target_t = dsk_target_field.getCompoundType()

    # Add to meta struct array: Source
    (source_gen_add_all, create) = mem_source_t.getOrAddTagInheritingRes(
        GenerateAddToAllMetaStructArraysFunction,
        dsk_type = dsk_source_t,
        mem_type = mem_source_t)

    if create:
        source_postconv = dsk_source_t.getOrAddTagInheriting(GeneratePostConversionFunction)
        source_postconv.addStep(FunctionTagStep(source_gen_add_all))

    # Join SOURCE
    join_sources = mem_source_t.getOrAddTagInheriting(JoinSources)
    js = JoinSource(dsk_src_field, mem_ptr_field, null_allowed)

    if not join_sources.hasSource(js):
        join_sources.addSource(js)

    if not meta_types.hasTypeName(js.getMetaType().getName()):
        meta_types.addType(js.getMetaType())

    # Add join source as meta mapping
    meta_sources = mem_source_t.getOrAddTagInheriting(DskToMetaSources)

    if not meta_sources.hasSource(js):
        meta_sources.addSource(js)

    # Add to meta struct array: target
    (target_gen_add_all, create) = mem_target_type.getOrAddTagInheritingRes(
        GenerateAddToAllMetaStructArraysFunction,
        dsk_type = dsk_target_t,
        mem_type = mem_target_type)

    if create:
        target_postconv = dsk_target_t.getOrAddTagInheriting(GeneratePostConversionFunction)
        target_postconv.addStep(FunctionTagStep(target_gen_add_all))

    # Join TARGET
    join_targets = mem_target_type.getOrAddTagInheriting(JoinTargets)
    jt = JoinTarget(dsk_target_field, mem_target_type)

    if not join_targets.hasTarget(jt):
        join_targets.addTarget(jt)

    if not meta_types.hasTypeName(jt.getMetaType().getName()):
        meta_types.addType(jt.getMetaType())

    meta_sources = mem_target_type.getOrAddTagInheriting(DskToMetaSources)

    if not meta_sources.hasSource(jt):
        meta_sources.addSource(jt)

    # Add Matching function

    (source_gen_match_all, create) = mem_source_t.getOrAddTagInheritingRes(
        GenerateMatchAllMetaStructArraysFunction)

    if create:
        source_fin = mem_source_t.getOrAddTagInheriting(GenerateFinalizeFunction)
        source_fin.addStep(FunctionTagStep(source_gen_match_all))

    js.setAssociatedJoinTarget(jt)
    jt.setAssociatedJoinSource(js)
