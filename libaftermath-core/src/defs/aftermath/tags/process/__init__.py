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

from aftermath.tags import Tag, \
    FunctionTag, \
    TagWithSteps, \
    TemplatedGenerateFunctionTag
import aftermath
from aftermath.util import enforce_type

class ProcessFunction(FunctionTag):
    """Indicates that postprocessing is needed after conversion from on-disk to
    in-memory format. The subtags indicate individual processing steps that
    should be carried out in their specified order."""

    def __init__(self, function_name = None):
        super(ProcessFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_process")

class GenerateProcessFunction(TemplatedGenerateFunctionTag,
                              TagWithSteps,
                              ProcessFunction):
    """Generates a process function"""

    def __init__(self, function_name = None, steps = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.ProcessFunction)

        TagWithSteps.__init__(self, steps)
        ProcessFunction.__init__(self, function_name = function_name)

class TraceMinMaxTimestampCompoundScan(FunctionTag):
    """Indicates that the tagged compound type has a function that updates the
    minimum and maximum timestamp of a trace from its fields.
    """

    def __init__(self, function_name = None):
        super(TraceMinMaxTimestampCompoundScan, self).__init__(
            function_name = function_name,
            default_suffix = "_update_min_max_timestamp")

class CheckGenerateTraceMinMaxTimestampCompoundScan(Tag):
    """Presence of this tag indicates that the type's finalize() function should
    check if the type references timestamps and, if it does, add appropriate
    tags to generate and call the minimum and maximum timestamp update function
    during processing of an instance of the type.
    """
    pass

class GenerateTraceMinMaxTimestampCompoundScan(TemplatedGenerateFunctionTag,
                                               TraceMinMaxTimestampCompoundScan):
    """Scans a compound type recursively for timestamps and generates code updating
    the minimum / maximum timestamp of a trace from the values"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.process.TraceMinMaxTimestampCompoundScan)

        TraceMinMaxTimestampCompoundScan.__init__(
            self, function_name = function_name)

    def __findTimestampsCompoundRec(self, t, types_checked = None):
        ret = []

        if types_checked is None:
            types_checked = []

        types_checked.append(t)

        if t.isCompound():
            for field in t.getFields():
                if field.getType() == aftermath.types.base.am_timestamp_t:
                    ret.append(field.getName())
                elif field.getType().isCompound():
                    if (not field.isPointer() or field.isOwned()) and \
                       (not field.getType() in types_checked):
                        # Recursively descend into compound field
                        rec = self.__findTimestampsCompoundRec(field.getType(),
                                                               types_checked)

                        operator = "->" if field.isPointer() else "."
                        rec = map(lambda e: field.getName() + operator + e, rec)
                        ret.extend(rec)

        return ret

    def setType(self, t):
        enforce_type(t, aftermath.types.CompoundType)

        TemplatedGenerateFunctionTag.setType(self, t)
        self.__accessors = self.__findTimestampsCompoundRec(t)

    def getAccessors(self):
        """Returns a list of strings, each providing access to a timestamp field when
        applied to the structure of the tag's associated type (e.g.,
        ["interval.start", "interval.end"] for a compound type that has a field
        named interval).
        """
        return self.__accessors
