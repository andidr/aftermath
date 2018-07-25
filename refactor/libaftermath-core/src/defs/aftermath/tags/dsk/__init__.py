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

from aftermath.tags import Tag, FunctionTag, GenerateFunctionTag, TemplatedGenerateFunctionTag, Compound
from aftermath.util import enforce_type
import aftermath

class ReadFunction(FunctionTag):
    """Function reading an instance of the associated type from disk"""

    def __init__(self, function_name = None):
        super(ReadFunction, self).__init__(function_name = function_name,
                                           default_suffix = "_read")

class GenerateReadFunction(TemplatedGenerateFunctionTag, ReadFunction):
    """Generate a ReadFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.ReadFunction)
        ReadFunction.__init__(self, function_name = function_name)

class DumpStdoutFunction(FunctionTag):
    """Function dumping the contents of an instance of the associated on-disk
    type to stdout"""

    def __init__(self, function_name = None):
        super(DumpStdoutFunction, self).__init__(function_name = function_name,
                                                 default_suffix = "_dump_stdout")

class GenerateDumpStdoutFunction(TemplatedGenerateFunctionTag,
                                 DumpStdoutFunction):
    """Generate a DumpStdoutFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.DumpStdoutFunction)
        DumpStdoutFunction.__init__(self, function_name = function_name)

class WriteFunction(FunctionTag):
    """Function writing an instance of the associated type to disk."""

    def __init__(self, function_name = None, has_type_param = False):
        """If `has_type_param` is true, an extra parameter with the numerical
        value for the on-disk frame is required to invoke this function."""

        super(WriteFunction, self).__init__(function_name = function_name,
                                            default_suffix = "_write")
        self.__has_type_param = has_type_param

    def hasTypeParam(self):
        return self.__has_type_param

    def setTypeParam(self, b):
        enforce_type(b, bool)

        self.__has_type_param = b

class GenerateWriteFunction(TemplatedGenerateFunctionTag, WriteFunction):
    """Generate a WriteFunction"""

    def __init__(self, function_name = None, has_type_param = False):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.WriteFunction)
        WriteFunction.__init__(self,
                               function_name = function_name,
                               has_type_param = has_type_param)

class LoadFunction(FunctionTag):
    """Function that loads one instance of the associated type from disk, then
    invoking the assertion and postprocessing function (if defined for the
    type)"""

    def __init__(self, function_name = None):
        super(LoadFunction, self).__init__(function_name = function_name,
                                           default_suffix = "_load")

class GenerateLoadFunction(TemplatedGenerateFunctionTag, LoadFunction):
    """Generate a LoadFunction"""

    def __init__(self, function_name = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.dsk.LoadFunction)
        LoadFunction.__init__(self, function_name = function_name)

class Frame(Tag):
    """Indicates that an on-disk structure is a frame (a data structure preceded by
    a numerical identifier for it's type)."""

    pass
