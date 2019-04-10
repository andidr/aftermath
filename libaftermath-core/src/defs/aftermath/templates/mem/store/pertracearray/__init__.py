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

from aftermath.templates import FunctionTemplate, \
    Jinja2FileTemplate, \
    gen_function_file_template_class_int_ctx

from aftermath.util import enforce_type
import aftermath.tags as tags
import aftermath
import os

from aftermath.types import Field, FieldList

# A template implementing
# aftermath.tags.mem.store.pertracearray.GenerateDestroyAllArraysFunction
DestroyAllArraysFunction = gen_function_file_template_class_int_ctx(
    class_name = "DestroyAllArraysFunction",
    required_tags = {
        "gen_tag" : tags.mem.store.pertracearray.GenerateDestroyAllArraysFunction
    },
    directory = os.path.dirname(__file__))

class BaseAppendFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template generating a function that appends an element to a
    per-event-collection array, creating the array if necessary.
    """

    def __init__(self, tpl_file, reqtags, mem_type):
        Jinja2FileTemplate.__init__(self, tpl_file)

        tag = reqtags["tag"]
        gen_tag = reqtags["gen_tag"]

        FunctionTemplate.__init__(
            self,
            function_name = tag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "e",
                      field_type = mem_type,
                      is_pointer = True)
            ]))

        if gen_tag.getTraceArrayIdent():
            trace_array_ident = gen_tag.getTraceArrayIdent()
        else:
            trace_array_ident = mem_type.getIdent()

        if gen_tag.getTraceArrayStructName():
            trace_array_struct_name = gen_tag.getEventArrayStructName()
        else:
            trace_array_struct_name = mem_type.getName()+"_array"

        self.addDefaultArguments( \
            mem_type = mem_type, \
            trace_array_ident = trace_array_ident, \
            trace_array_struct_name = trace_array_struct_name, \
            **reqtags)

class AppendFunction(BaseAppendFunction):
    """A template implementing
    aftermath.tags.mem.store.pertracearray.GenerateAppendFunction"""

    def __init__(self, mem_type):
        reqtags = self.requireTags(mem_type, {
            "tag" : tags.mem.store.pertracearray.AppendFunction,
            "gen_tag" : tags.mem.store.pertracearray.GenerateAppendFunction
        })

        BaseAppendFunction.__init__(
            self,
            "AppendFunction.tpl.c",
            reqtags,
            mem_type)

class AppendAndSetIndexFunction(BaseAppendFunction):
    """A template implementing
    aftermath.tags.mem.store.pertracearray.GenerateAppendAndSetIndexFunction"""

    def __init__(self, mem_type):
        reqtags = self.requireTags(mem_type, {
            "tag" : tags.mem.store.pertracearray.AppendAndSetIndexFunction,
            "gen_tag" : tags.mem.store.pertracearray.GenerateAppendAndSetIndexFunction
        })

        BaseAppendFunction.__init__(
            self,
            "AppendAndSetIndexFunction.tpl.c",
            reqtags,
            mem_type)

        enforce_type(mem_type, aftermath.types.CompoundType)

        if not mem_type.getFields().hasFieldByName("idx") or \
           not mem_type.getFields().getFieldByName("idx").getType() == \
           aftermath.types.builtin.size_t:
            raise Exception("Type must have a field named 'idx' of type size_t.")
