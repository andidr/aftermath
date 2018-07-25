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
    GenerateFunctionTag, \
    TagWithSteps, \
    TagStep, \
    TemplatedGenerateFunctionTag
from aftermath.util import enforce_type
import aftermath
import re

class AssertFunction(FunctionTag):
    """Function verifying the integrity of a single instance of the associated
    type"""

    def __init__(self, function_name = None):
        super(AssertFunction, self).__init__(function_name = function_name,
                                             default_suffix = "_assert")

class AssertStep(TagStep):
    """Single step for an assertion function"""

    def __init__(self, expression, error_message):
        """`expression` is a string containing a C expression that can be used as
        an expression for an if statement. A substring in double braces is
        replaced by an expression accessing the field whose name is passed in
        double braces (e.g., in a context where s is the pointer to an instance
        of the structure to be asserted, "{{interval.start}} > 100" would be
        replaced by "s->interval.start > 100").

        `error_message` is a string describing the situation in which the assertion
        failed

        """

        enforce_type(expression, str)
        enforce_type(error_message, str)

        self.__expression = expression
        self.__error_message = error_message

    def getExpression(self, t, instance_name, is_pointer):
        if is_pointer:
            acc = instance_name + "->"
        else:
            acc = instance_name + "."

        return re.sub(r"\{\{([^}]+)\}\}", acc + "\\1", self.__expression)

    def getErrorMessage(self):
        return self.__error_message


class GenerateAssertFunction(TemplatedGenerateFunctionTag,
                             TagWithSteps,
                             AssertFunction):
    """Generate an AssertFunction"""

    def __init__(self, function_name = None, steps = None):
        TemplatedGenerateFunctionTag.__init__(
            self,
            template_type = aftermath.templates.assertion.AssertFunction)

        TagWithSteps.__init__(self, steps)
        AssertFunction.__init__(self, function_name = function_name)
