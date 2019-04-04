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

from aftermath.tags import Tag, FunctionTagStep
from aftermath.tags.dsk.tomem import GenerateSortMetaStructArrayFunction
from aftermath.tags.postprocess import GeneratePostprocessFunction
from aftermath.types import Type, Field, FieldList, CompoundType
from aftermath.util import enforce_type, AbstractFunction
import aftermath

class DskToMetaSource(object):
    """Abstract type with common operations for JoinSource and JoinTarget; Both
    JoinSource and JoinTarget are modeled as DskToMetaSources, since they both
    represent a relation of a field of an on-disk structure to an in-memory type
    (the ID of the on-disk data structure is stored in the instance of the
    meta-type).
    """

    def __hash__(self):
        return hash(self.getMetaType().getName())

    def __eq__(self, other):
        return self.getMetaType().getName() == other.getMetaType().getName()

    @AbstractFunction
    def getDskField(self):
        pass

    @AbstractFunction
    def getDskType(self):
        pass

    @AbstractFunction
    def getMemType(self):
        pass

    @AbstractFunction
    def getMetaType(self):
        pass

class JoinSource(DskToMetaSource):
    """A single source for a join (@see aftermath.relations.join.make_join)."""

    def __init__(self, dsk_field, mem_field, null_allowed):
        """`dsk_field` is the field of the on-disk data structure associated to the
        join's source data structure that contains the value that is to be
        matched with the on-disk data structure associated to the target data
        structure.

        The Field instance `mem_field` is the field of the actual data structure
        that points to the target data structure.

        If `null_allowed` is true, the code matching the data structures does
        not fail if for a source data structure no target is found.
        """

        enforce_type(dsk_field, Field)
        enforce_type(mem_field, Field)
        enforce_type(null_allowed, bool)

        self.__dsk_field = dsk_field
        self.__mem_field = mem_field
        self.__meta_type = None
        self.__associated_join_target = None
        self.__null_allowed = null_allowed

    def getMemField(self):
        return self.__mem_field

    def getMemType(self):
        return self.__mem_field.getCompoundType()

    def getDskField(self):
        return self.__dsk_field

    def getDskType(self):
        return self.__dsk_field.getCompoundType()

    def getMetaType(self):
        if self.__meta_type is None:
            self.__buildMetaType()

        return self.__meta_type

    def getAssociatedJoinTarget(self):
        return self.__associated_join_target

    def setAssociatedJoinTarget(self, tgt):
        enforce_type(tgt, JoinTarget)
        self.__associated_join_target = tgt

    def nullAllowed(self):
        return self.__null_allowed

    def __buildMetaType(self):
        # Build source meta type
        #
        #   struct <mem_source_t>__meta_<pointer_name> {
        #     <dsk_source_field> id;
        #   };
        #
        self.__meta_type = CompoundType(
            name = self.getMemType().getName() +
                "__meta_join_src_" +
                self.getDskField().getName(),

            entity = "Meta type for " + self.getDskType().getName(),

            comment = "Meta type for " + self.getMemType().getName() + " for " +
                        "pointer '" + self.getMemField().getName() + "'",

            ident = self.getMemType().getIdent() +
                        ":::meta::join::src::" +
                        self.__dsk_field.getName(),

            fields = FieldList([
                Field(
                    name = "id",
                    type = self.__dsk_field.getType(),
                    comment = "Value from " + self.getDskType().getName() + \
                                "." + self.__dsk_field.getName())
            ]))

        if self.getMemType().getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction):
            self.__meta_type.addTags(aftermath.tags.mem.store.pertracearray.GenerateAppendFunction())
            daf = aftermath.tags.mem.store.pertracearray.GenerateDestroyAllArraysFunction()
        elif self.getMemType().getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction):
            self.__meta_type.addTags(aftermath.tags.mem.store.pereventcollectionarray.GenerateAppendFunction())
            daf = aftermath.tags.mem.store.pereventcollectionarray.GenerateDestroyAllArraysFunction()
        elif self.getMemType().getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction):
            self.__meta_type.addTags(aftermath.tags.mem.store.pereventcollectionsubarray.GenerateAppendFunction())
            daf = aftermath.tags.mem.store.pereventcollectionsubarray.GenerateDestroyAllArraysFunction()
        else:
            raise Exception("Unknown storage class")

        self.__meta_type.addTags(daf)

        tdf = self.getMemType().getOrAddTagInheriting(aftermath.tags.teardown.GenerateTeardownFunction)
        tdf.addStep(FunctionTagStep(daf))

class JoinTarget(DskToMetaSource):
    """A single target for a join (@see aftermath.relations.join.make_join)."""

    def __init__(self, dsk_field, mem_type):
        """`dsk_field` is the field of the on-disk data structure associated to the
        join's target data structure that contains the value that is to be
        matched with the on-disk data structure associated to the source data
        structure.

        The Field instance `mem_type` is the type of actual data structure
        targeted by the join (which is pointed to by the pointer field of the
        source data structure).
        """

        enforce_type(dsk_field, Field)
        enforce_type(mem_type, Type)

        self.__dsk_field = dsk_field
        self.__mem_type = mem_type
        self.__meta_type = None
        self.__associated_join_source = None

    def getMemType(self):
        return self.__mem_type

    def getDskField(self):
        return self.__dsk_field

    def getDskType(self):
        return self.__dsk_field.getCompoundType()

    def getMetaType(self):
        if self.__meta_type is None:
            self.__buildMetaType()

        return self.__meta_type

    def getAssociatedJoinSource(self):
        return self.__associated_join_source

    def setAssociatedJoinSource(self, src):
        enforce_type(src, JoinSource)
        self.__associated_join_source = src

    def __buildMetaType(self):
        # Build target meta type
        #
        #   struct <mem_source_t>__meta_<pointer_name> {
        #     <dsk_source_field> id;
        #     size_t idx;
        #   };
        #

        meta_sort_tag = GenerateSortMetaStructArrayFunction()

        self.__meta_type = CompoundType(
            name = self.getMemType().getName() +
                "__meta_join_tgt_" +
                self.getDskField().getName(),

            entity = "Meta type for " + self.getDskType().getName(),

            comment = "Meta type for " + self.getMemType().getName() + " for " +
                        "field '" + self.getDskField().getName() + "' of the " +
                        "corresponding on-disk data structure "+
                        "'"+ self.getDskType().getName()+"'",

            ident = self.getMemType().getIdent() +
                        ":::meta::join::tgt::" +
                        self.__dsk_field.getName(),

            fields = FieldList([
                Field(
                    name = "id",
                    type = self.__dsk_field.getType(),
                    comment = "Value from " + self.getDskType().getName() + \
                                "." + self.__dsk_field.getName()),
                Field(
                    name = "idx",
                    type = aftermath.types.builtin.size_t,
                    comment = "Array index of the target structure associated " +
                    "to the value")
            ]),
            tags = [
                meta_sort_tag,
                GeneratePostprocessFunction(steps = [
                    FunctionTagStep(meta_sort_tag)
                ])
            ])

        if self.getMemType().getTagInheriting(aftermath.tags.mem.store.pertracearray.AppendFunction):
            self.__meta_type.addTags(aftermath.tags.mem.store.pertracearray.GenerateAppendAndSetIndexFunction())
            daf = aftermath.tags.mem.store.pertracearray.GenerateDestroyAllArraysFunction()
        elif self.getMemType().getTagInheriting(aftermath.tags.mem.store.pereventcollectionarray.AppendFunction):
            self.__meta_type.addTags(aftermath.tags.mem.store.pereventcollectionarray.GenerateAppendAndSetIndexFunction())
            daf = aftermath.tags.mem.store.pereventcollectionarray.GenerateDestroyAllArraysFunction()
        elif self.getMemType().getTagInheriting(aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction):
            self.__meta_type.addTags(aftermath.tags.mem.store.pereventcollectionsubarray.GenerateAppendFunction())
            daf = aftermath.tags.mem.store.pereventcollectionsubarray.GenerateDestroyAllArraysFunction()
        else:
            raise Exception("Unknown storage class")

        self.__meta_type.addTags(daf)

        tdf = self.getMemType().getOrAddTagInheriting(aftermath.tags.teardown.GenerateTeardownFunction)
        tdf.addStep(FunctionTagStep(daf))

class DskToMetaSources(Tag):
    """A tag holding a list of on-disk to meta structure sources; Sources refers
    to the on-disk to in-memory meta-type relation and NOT the join relation!

    This list thus specified what needs to be copied from an on-disk types to
    meta-data types and NOT how join sources and join targets relate.
    """

    def __init__(self):
        self.__sources = []
        self.__sources_hashed = {}

    def getSources(self):
        return self.__sources

    def addSource(self, src):
        enforce_type(src, DskToMetaSource)

        if self.hasSource(src):
            raise Exception("Source for on-disk field " +
                            "'" + src.getDskField().getName() + "' " +
                            "of type " +
                            "'" + src.getDskField().getCompoundType().getName() + "' " +
                            "already added")

        self.__sources.append(src)
        self.__sources_hashed[src] = src

    def hasSource(self, src):
        return src in self.__sources_hashed.keys()

class JoinTargets(Tag):
    """A tag holding a list of join targets"""

    def __init__(self):
        self.__targets = []
        self.__targets_hashed = {}

    def getTargets(self):
        return self.__targets

    def addTarget(self, tgt):
        enforce_type(tgt, JoinTarget)

        if self.hasTarget(tgt):
            raise Exception("Target for on-disk field " +
                            "'" + tgt.getDskField().getName() + "' " +
                            "of type " +
                            "'" + tgt.getDskField().getCompoundType().getName() + "' " +
                            "already added")

        self.__targets.append(tgt)
        self.__targets_hashed[tgt] = tgt

    def hasTarget(self, tgt):
        """Returns True if this list of join targets already has the join target
        given as the parameter."""

        return tgt in self.__targets_hashed.keys()

class JoinSources(Tag):
    """A tag holding a list of join sources"""

    def __init__(self):
        self.__sources = []
        self.__sources_hashed = {}

    def getSources(self):
        return self.__sources

    def addSource(self, src):
        enforce_type(src, JoinSource)

        if self.hasSource(src):
            raise Exception("Source for on-disk field " +
                            "'" + src.getDskField().getName() + "' " +
                            "of type " +
                            "'" + src.getDskField().getCompoundType().getName() + "' " +
                            "already added")

        self.__sources.append(src)
        self.__sources_hashed[src] = src

    def hasSource(self, src):
        """Returns True if this list of join sources already has the join source
        given as the parameter."""

        return src in self.__sources_hashed.keys()
