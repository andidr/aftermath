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

from aftermath.templates import FunctionTemplate, Jinja2FileTemplate
from aftermath import tags
from aftermath.types import FieldList, Field
import aftermath

class ReadFunction(FunctionTemplate, Jinja2FileTemplate):
    """Template implementing aftermath.tags.dsk.GenerateReadFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "ReadFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "tag" : tags.dsk.GenerateReadFunction
        })

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      type = dsk_type,
                      is_pointer = True)
            ]))
        self.addDefaultArguments(dsk_type = dsk_type, **reqtags)

class DumpStdoutFunction(FunctionTemplate, Jinja2FileTemplate):
    """Template implementing aftermath.tags.dsk.GenerateDumpStdoutFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "DumpStdoutFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "tag" : tags.dsk.GenerateDumpStdoutFunction
        })

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      type = dsk_type,
                      is_pointer = True),
                Field(name = "indent",
                      type = aftermath.types.builtin.size_t),
                Field(name = "next_indent",
                      type = aftermath.types.builtin.size_t)
            ]))

        self.addDefaultArguments(dsk_type = dsk_type, **reqtags)

class WriteFunction(FunctionTemplate, Jinja2FileTemplate):
    """Template implementing aftermath.tags.dsk.GenerateWriteFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "WriteFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "gen_tag" : tags.dsk.GenerateWriteFunction
        })

        if reqtags["gen_tag"].hasTypeParam():
            tp = [ Field(name = "type_id", type = aftermath.types.builtin.uint32_t) ]
        else:
            tp = []

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["gen_tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            arglist = FieldList(
                [ Field(name = "ctx",
                        type = aftermath.types.aux.am_io_context,
                        is_pointer = True) ] +
                tp +
                [ Field(name = "dsk",
                        type = dsk_type,
                        is_pointer = True,
                        is_const = True) ]))

        self.addDefaultArguments(dsk_type = dsk_type, **reqtags)

class LoadFunction(FunctionTemplate, Jinja2FileTemplate):
    """Template implementing aftermath.tags.dsk.GenerateLoadFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "LoadFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "load_tag" : tags.dsk.LoadFunction,
            "read_tag" : tags.dsk.ReadFunction
        })

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["load_tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      type = aftermath.types.aux.am_io_context,
                      is_pointer = True)
            ]))

        self.addDefaultArguments(dsk_type = dsk_type, **reqtags)


class ProcessFunction(FunctionTemplate, Jinja2FileTemplate):
    """Template implementing aftermath.tags.dsk.GenerateProcessFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "ProcessFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "gen_tag" : tags.process.GenerateProcessFunction,
        })

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["gen_tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "e",
                      type = dsk_type,
                      is_pointer = True)
            ]))
        self.addDefaultArguments(dsk_type = dsk_type, **reqtags)
