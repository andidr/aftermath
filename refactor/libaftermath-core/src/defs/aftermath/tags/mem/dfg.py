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

from aftermath.tags import Tag
from aftermath.util import enforce_type
import aftermath

class DeclareConstPointerType(Tag):
    """Declares a DFG type for constant pointers to the type"""

    def __init__(self):
        super(DeclareConstPointerType, self).__init__()

class DeclareEventMappingOverlappingIntervalExtractionNode(Tag):
    """Declares a DFG node type that extracts constant pointers to all instances of
    the type in all event mappings present at the input port that overlap with
    at least one of the intervals at another input port.
    """

    def __init__(self, stripname_plural, port_name, include_file,
                 array_struct_name = None,
                 title_hrplural_cap = None):
        """`stripname_plural` gets appended to the symbol for the generated node
        and should be equal to the name of the type without the am_ prefix in
        plural form (e.g., "state_events" for the type am_state_event).

        `port_name` is the name of the output port of the node

        `include_file` is an include file specification that can be passed to
        the #include directive (i.e., a filename in double quotes "..." or in
        angle backets <...>)

        `array_struct_name` is a string containing the array prefix used for the
        typed array for the tagged type. If None, the name is derived
        automatically by appending "_array" to the type's name.

        `title_hrplural_cap` is the capitalized, second part of the node title
        in human-redable form, e.g., "State Events". If None, this is derived
        from the entity name.
        """
        super(DeclareEventMappingOverlappingIntervalExtractionNode, self).__init__()

        self.__array_struct_name = array_struct_name
        self.__stripname_plural = stripname_plural
        self.__port_name = port_name
        self.__include_file = include_file
        self.__title_hrplural_cap = title_hrplural_cap

    def getArrayStructName(self):
        if self.__array_struct_name is None:
            return self.getType().getName()+"_array"
        else:
            return self.__array_struct_name

    def getStripNamePlural(self):
        return self.__stripname_plural

    def getPortName(self):
        return self.__port_name

    def getIncludeFile(self):
        return self.__include_file

    def getCapitalizedHumanReadablePlural(self):
        if self.__title_hrplural_cap is None:
            return self.getType().getEntity()+"s"
        else:
            return self.__title_hrplural_cap
