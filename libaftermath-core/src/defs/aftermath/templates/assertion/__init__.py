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

class AssertFunction(FunctionTemplate, Jinja2FileTemplate):
    """A template implementing aftermath.tags.assertion.GenerateAssertFunction"""

    def __init__(self, t):
        Jinja2FileTemplate.__init__(self, "AssertFunction.tpl.c")

        reqtags = self.requireTags(t, {
            "gen_tag" : tags.assertion.GenerateAssertFunction,
        })

        FunctionTemplate.__init__(
            self,
            function_name = reqtags["gen_tag"].getFunctionName(),
            return_type = aftermath.types.builtin.int,
            inline = True,
            arglist = FieldList([
                Field(name = "ctx",
                      field_type = aftermath.types.aux.am_io_context,
                      is_pointer = True),
                Field(name = "e",
                      field_type = t,
                      is_pointer = True,
                      is_const = True)
            ]))
        self.addDefaultArguments(t = t, **reqtags)
