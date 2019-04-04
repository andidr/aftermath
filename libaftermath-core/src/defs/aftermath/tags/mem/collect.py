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

from aftermath.tags import Tag, FunctionTag, TemplatedGenerateFunctionTag
from aftermath.util import enforce_type
from aftermath.types import Field
import aftermath

class SelfModeVal:
    def __init__(self, intval):
        enforce_type(intval, int)

        if intval < 0 or intval > 4:
            raise Exception("Illegal value for SelfMode")

        self.intval = intval

class SelfMode:
    ALLOWED = SelfModeVal(1)
    IGNORE = SelfModeVal(2)
    ERROR = SelfModeVal(3)
    SETNULL = SelfModeVal(4)

class CollectSource(object):
    """A single source for a collect operation (@see
    aftermath.relations.collect.make_collect_pointers)."""

    def __init__(self,
                 ptr_field,
                 null_allowed = True,
                 self_mode = SelfMode.ALLOWED):
        """`ptr_field` the field that points to the yarget structure.

        If `null_allowed` is true, the code matching the data structures does
        not fail if for a source data structure no target is found.

        If self_mode is SelfMode.ALLOWED, self-references are allowed; if
        self_mode is SelfMode.IGNORE an instance is not added to the array of
        the structure it points to if this is the same as the instance itself;
        If self_mode is SelfMode.ERROR, this is even considered an error. If
        self_mode is SelfMode.SETNULL, the pointer is set to NULL.
        """

        enforce_type(ptr_field, Field)
        enforce_type(null_allowed, bool)
        enforce_type(self_mode, SelfModeVal)

        self.__ptr_field = ptr_field
        self.__null_allowed = null_allowed
        self.__associated_target = None
        self.__self_mode = self_mode

    def getPtrField(self):
        return self.__ptr_field

    def getType(self):
        return self.__ptr_field.getCompoundType()

    def getAssociatedTarget(self):
        return self.__associated_target

    def setAssociatedTarget(self, tgt):
        enforce_type(tgt, CollectTarget)
        self.__associated_target = tgt

    def nullAllowed(self):
        return self.__null_allowed

    def getSelfMode(self):
        return self.__self_mode

    def __hash__(self):
        return hash(self.getPtrField().getName())

    def __eq__(self, other):
        return self.getPtrField() == other.getPtrField()

class CollectTarget(object):
    """A single target for a collect operation (@see
    aftermath.relations.collect.make_collect_pointers)."""

    def __init__(self, num_field, array_field):
        """`num_field` the field that counts the number of pointers in the array
        of pointers to the sources.

        `array_field` the field that collects the pointers to the sources.
        """

        enforce_type(array_field, Field)
        enforce_type(num_field, Field)

        self.__array_field = array_field
        self.__num_field = num_field

    def getArrayField(self):
        return self.__array_field

    def getNumField(self):
        return self.__num_field

    def getType(self):
        return self.__num_field.getCompoundType()

    def getAssociatedSource(self):
        return self.__associated_source

    def setAssociatedSource(self, src):
        enforce_type(src, CollectSource)
        self.__associated_source = src

    def __hash__(self):
        return hash(self.getNumField().getName())

    def __eq__(self, other):
        return self.getNumField() == other.getNumField()

class CollectSources(Tag):
    """A tag holding a list of collect sources"""

    def __init__(self):
        self.__sources = []
        self.__sources_hashed = {}

    def getSources(self):
        return self.__sources

    def addSource(self, src):
        enforce_type(src, CollectSource)

        if self.hasSource(src):
            raise Exception("Collect source for field " +
                            "'" + src.getPtrField().getName() + "' " +
                            "of type " +
                            "'" + src.getType().getName() + "' " +
                            "already added")

        self.__sources.append(src)
        self.__sources_hashed[src] = src

    def hasSource(self, src):
        return src in self.__sources_hashed.keys()

class CollectTargets(Tag):
    """A tag holding a list of join targets"""

    def __init__(self):
        self.__targets = []
        self.__targets_hashed = {}

    def getTargets(self):
        return self.__targets

    def addTarget(self, tgt):
        enforce_type(tgt, CollectTarget)

        if self.hasTarget(tgt):
            raise Exception("Collect target for field " +
                            "'" + src.getArrayField().getName() + "' " +
                            "of type " +
                            "'" + src.getType().getName() + "' " +
                            "already added")

        self.__targets.append(tgt)
        self.__targets_hashed[tgt] = tgt

    def hasTarget(self, tgt):
        """Returns True if this list of collect targets already has the colelct
        target given as the parameter."""

        return tgt in self.__targets_hashed.keys()

class ProcessCollectSourcesFunction(FunctionTag):
    """Adds all instances of the type to the instances they point to as defined
    in the CollectSources tag"""

    def __init__(self, function_name = None):
        super(ProcessCollectSourcesFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_process_collect_sources")

class GenerateProcessCollectSourcesFunction(TemplatedGenerateFunctionTag,
                                            ProcessCollectSourcesFunction):
    """Generate a ProcessCollectSourcesFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.mem.GenerateProcessCollectSourcesFunction)

        ProcessCollectSourcesFunction.__init__(
            self, function_name = function_name)
