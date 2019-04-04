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

from aftermath.util import enforce_type
from aftermath.tags import FunctionTag, \
    TagWithSteps, \
    TemplatedGenerateFunctionTag

import aftermath

class PostprocessFunction(FunctionTag):
    """A postprocess function is invoked individually for each instance of the type
    as soon as it has been constructed (e.g., for an on-disk type after it has
    been loaded from disk and converted to host endianness). This is however,
    before the instance has reached its final location in memory.
    """

    def __init__(self, function_name = None):
        super(PostprocessFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_postprocess")

class GeneratePostprocessFunction(PostprocessFunction,
                                  TemplatedGenerateFunctionTag,
                                  TagWithSteps):
    """Generate a PostprocessFunction"""

    def __init__(self, function_name = None, steps = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.postprocess.PostprocessFunction)

        TagWithSteps.__init__(self, steps)
        PostprocessFunction.__init__(self, function_name = function_name)

