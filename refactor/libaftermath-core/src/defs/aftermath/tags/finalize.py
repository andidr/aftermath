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

from aftermath.tags import FunctionTag, \
    TagWithSteps, \
    TemplatedGenerateFunctionTag
from aftermath.util import enforce_type
import aftermath

class FinalizeFunction(FunctionTag):
    """The final function is executed once for all instances of the type after the
    entire trace has been loaded and postprocessed needs to be called for the
    type. In-memory types will have reached their final address.
    """

    def __init__(self, function_name = None):
        super(FinalizeFunction, self).__init__(
            function_name = function_name,
            default_suffix = "_finalize")

class GenerateFinalizeFunction(TemplatedGenerateFunctionTag,
                               TagWithSteps,
                               FinalizeFunction):
    """Generate a finalize function. The subtags indicate individual steps that
    should be carried out in their specified order.
    """

    def __init__(self, function_name = None, steps = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.finalize.FinalizeFunction)
        TagWithSteps.__init__(self, steps)
        FinalizeFunction.__init__(self, function_name = function_name)
