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
    gen_function_file_template_class, \
    gen_function_file_template_class_int_ctx
from aftermath import tags
from aftermath.types import FieldList, Field
import aftermath
import os

class ConversionFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing tags.dsk.tomem.ConversionFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "ConversionFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "tag" : tags.dsk.tomem.ConversionFunction })

        tag = reqtags["tag"]
        mem_type = tag.getMemType()

        FunctionTemplate.__init__(
            self,
            function_name = tag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      field_type = dsk_type,
                      is_pointer = True),
                Field(name = "mem",
                      field_type = mem_type,
                      is_pointer = True)
            ]))

        self.addDefaultArguments(mem_type = mem_type, \
                                 dsk_type = dsk_type, \
                                 **reqtags)

class PostConversionFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing tags.dsk.tomem.GeneratePostConversionFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "PostConversionFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "gen_tag" : tags.dsk.tomem.GeneratePostConversionFunction,
            "conversion_tag" : tags.dsk.tomem.ConversionFunction
        })

        tag = reqtags["gen_tag"]
        conversion_tag = reqtags["conversion_tag"]
        mem_type = conversion_tag.getMemType()

        FunctionTemplate.__init__(
            self,
            function_name = tag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      field_type = dsk_type,
                      is_pointer = True),
                Field(name = "mem",
                      field_type = conversion_tag.getMemType(),
                      is_pointer = True)
            ]))

        self.addDefaultArguments(mem_type = mem_type, \
                                 dsk_type = dsk_type, \
                                 **reqtags)

class PerTraceArrayFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing tags.dsk.tomem.GeneratePerTraceArrayFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "PerTraceArrayFunction.tpl.c")

        reqtags_dsk = self.requireTags(dsk_type, {
            "gen_tag" : tags.dsk.tomem.GeneratePerTraceArrayFunction,
            "tomem_tag" : aftermath.tags.dsk.tomem.ConversionFunction })

        gen_tag = reqtags_dsk["gen_tag"]
        tomem_tag = reqtags_dsk["tomem_tag"]

        reqtags_mem = self.requireTags(tomem_tag.getMemType(), {
            "store_tag" : aftermath.tags.mem.store.pertracearray.AppendFunction
        })

        FunctionTemplate.__init__(
            self,
            function_name = gen_tag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      field_type = dsk_type,
                      is_pointer = True)
            ]))

        self.addDefaultArguments(dsk_type = dsk_type)
        self.addDefaultArguments(**reqtags_dsk)
        self.addDefaultArguments(**reqtags_mem)

class PerEventCollectionArrayFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing
    tags.dsk.tomem.GeneratePerEventCollectionArrayFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "PerEventCollectionArrayFunction.tpl.c")

        reqtags_dsk = self.requireTags(dsk_type, {
            "gen_tag" : tags.dsk.tomem.GeneratePerEventCollectionArrayFunction,
            "tomem_tag" : aftermath.tags.dsk.tomem.ConversionFunction
        })

        gen_tag = reqtags_dsk["gen_tag"]
        tomem_tag = reqtags_dsk["tomem_tag"]

        reqtags_mem = self.requireTags(tomem_tag.getMemType(), {
            "store_tag" : aftermath.tags.mem.store.pereventcollectionarray.AppendFunction
        })

        FunctionTemplate.__init__(
            self,
            function_name = gen_tag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      field_type = dsk_type,
                      is_pointer = True)
            ]))

        self.addDefaultArguments(dsk_type = dsk_type)
        self.addDefaultArguments(**reqtags_dsk)
        self.addDefaultArguments(**reqtags_mem)

class PerEventCollectionSubArrayFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing
    tags.dsk.tomem.GeneratePerEventCollectionSubArrayFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self,
                                    "PerEventCollectionSubArrayFunction.tpl.c")

        reqtags_dsk = self.requireTags(dsk_type, {
            "gen_tag" : tags.dsk.tomem.GeneratePerEventCollectionSubArrayFunction,
            "tomem_tag" : aftermath.tags.dsk.tomem.ConversionFunction
        })

        gen_tag = reqtags_dsk["gen_tag"]
        tomem_tag = reqtags_dsk["tomem_tag"]

        reqtags_mem = self.requireTags(tomem_tag.getMemType(), {
            "store_tag" : aftermath.tags.mem.store.pereventcollectionsubarray.AppendFunction
        })

        FunctionTemplate.__init__(
            self,
            function_name = gen_tag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      field_type = dsk_type,
                      is_pointer = True)
            ]))

        self.addDefaultArguments(dsk_type = dsk_type)
        self.addDefaultArguments(**reqtags_dsk)
        self.addDefaultArguments(**reqtags_mem)

# Template implementing tags.dsk.tomem.GenerateSortMetaStructArrayFunction
SortMetaStructArrayFunction = gen_function_file_template_class_int_ctx(
    class_name = "SortMetaStructArrayFunction",
    required_tags = {
        "gen_tag" :
        tags.dsk.tomem.GenerateSortMetaStructArrayFunction
    },
    directory = os.path.dirname(__file__))

# Template implementing tags.dsk.tomem.GenerateMatchAllMetaStructArraysFunction
MatchAllMetaStructArraysFunction = gen_function_file_template_class_int_ctx(
    class_name = "MatchAllMetaStructArraysFunction",
    required_tags = {
        "gen_tag" : tags.dsk.tomem.GenerateMatchAllMetaStructArraysFunction,
        "sources_tag" : tags.dsk.tomem.join.JoinSources
    },
    directory = os.path.dirname(__file__))

class AddToAllMetaStructArraysFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing
    tags.dsk.tomem.GenerateAddToAllMetaStructArraysFunction"""

    def __init__(self, dsk_type):
        Jinja2FileTemplate.__init__(self, "AddToAllMetaStructArraysFunction.tpl.c")

        reqtags = self.requireTags(dsk_type, {
            "gen_tag" : tags.dsk.tomem.GenerateAddToAllMetaStructArraysFunction,
            "dsktometa_tag" : tags.dsk.tomem.join.DskToMetaSources
        })

        gen_tag = reqtags["gen_tag"]

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["gen_tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "dsk",
                      field_type = gen_tag.getDskType(),
                      is_pointer = True),
                Field(name = "mem",
                      field_type = gen_tag.getMemType(),
                      is_pointer = True)
            ]))
        self.addDefaultArguments(**reqtags)
