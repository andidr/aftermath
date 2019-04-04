#!/usr/bin/env python

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
from aftermath.util import enforce_type
from aftermath.types import FieldList, Field
from aftermath import tags
import aftermath

class TraceMinMaxTimestampCompoundScan(FunctionTemplate, Jinja2FileTemplate):
    """Template implementing
    tags.process.GenerateTraceMinMaxTimestampCompoundScan"""

    def __init__(self, t):
        Jinja2FileTemplate.__init__(
            self, "TraceMinMaxTimestampCompoundScan.tpl.c")

        enforce_type(t, aftermath.types.CompoundType)

        reqtags = self.requireTags(t, {
            "gen_tag" : tags.process.GenerateTraceMinMaxTimestampCompoundScan
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
                      type = t,
                      is_pointer = True)
            ]))

        self.addDefaultArguments(t = t, **reqtags)
