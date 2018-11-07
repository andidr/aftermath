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

from aftermath.types import TypeList, NonCompoundType, FixedWidthIntegerType, IntegerType, Type
from aftermath.util import enforce_type
from aftermath.tags import Destructor

import aftermath.types.builtin
import aftermath

class BaseType(NonCompoundType):
    """Non-compound, in-memory type that may be used for multiple programming
    models. The purpose of base types is to precisely define the characteristics
    of a data type that appears in multiple models and to prevent diverging
    definitions.

    A base aliases an existing type (e.g., a builtin C type with portable
    characteristics, such as an integer of a fixed width).
    """

    def __init__(self, name, entity, aliased_type, comment, *args, **kwargs):
        """`name`, `entity`, and `comment` are the same as for
        aftermath.types.Type.

        `aliased_type` is the existing type defining the characteristics of the
        base type.
        """
        enforce_type(aliased_type, Type)

        NonCompoundType.__init__(self,
                                 name = name,
                                 entity = entity,
                                 comment = comment,
                                 *args,
                                 **kwargs)

        self.__aliased_type = aliased_type

        self.addTags(
            aftermath.tags.dsk.GenerateWriteToBufferFunction())

    def getAliasedType(self):
        """Returns the type this type is an alias for"""

        return self.__aliased_type

class BaseTypeFixedWidthIntegerAlias(BaseType, FixedWidthIntegerType):
    """Base type for Aftermath which is an integer with an
    implementation-independent width in bits"""

    def __init__(self, name, entity, aliased_type, comment):
        """See BaseType.__init__"""

        enforce_type(aliased_type, FixedWidthIntegerType)

        BaseType.__init__(
            self,
            aliased_type = aliased_type,
            name = name,
            entity = entity,
            comment = comment)

        FixedWidthIntegerType.__init__(
            self,
            signed = aliased_type.isSigned(),
            num_bits = aliased_type.getNumBits(),
            name = name,
            entity = entity,
            comment = comment)

        self.setFormatStringSym(name.upper()+"_FMT_T")

am_bool_t = BaseType(
    name = "am_bool_t",
    entity = "boolean",
    comment = "Boolean value",
    aliased_type = aftermath.types.builtin.int)

am_timestamp_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_timestamp_t",
    entity = "timestamp",
    comment = "Time stamp expressed in cycles",
    aliased_type = aftermath.types.builtin.uint64_t)

am_counter_value_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_counter_value_t",
    entity = "counter value",
    comment = "Type for counter samples",
    aliased_type = aftermath.types.builtin.uint64_t)

am_event_collection_id_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_event_collection_id_t",
    entity = "collection ID",
    comment = "Numerical ID of an event collection",
    aliased_type = aftermath.types.builtin.uint32_t)

am_cpu_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_cpu_t",
    entity = "CPU ID",
    comment = "Numerical ID of a CPU",
    aliased_type = aftermath.types.builtin.uint32_t)

am_counter_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_counter_t",
    entity = "counter ID",
    comment = "Numerical ID of a counter",
    aliased_type = aftermath.types.builtin.uint32_t)

am_nanoseconds_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_nanoseconds_t",
    entity = "inanoseconds",
    comment = "Time stamp expressed in nanoseconds",
    aliased_type = aftermath.types.builtin.uint64_t)

am_hierarchy_id_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_hierarchy_id_t",
    entity = "hierarchy ID",
    comment = "Numerical ID of a hierarchy",
    aliased_type = aftermath.types.builtin.uint32_t)

am_hierarchy_node_id_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_hierarchy_node_id_t",
    entity = "hierarchy node ID",
    comment = "Numerical ID of a hierarchy node",
    aliased_type = aftermath.types.builtin.uint32_t)

am_state_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_state_t",
    entity = "state",
    comment = "Numerical ID of a state",
    aliased_type = aftermath.types.builtin.uint32_t)

am_source_line_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_source_line_t",
    entity = "source line",
    comment = "Zero-index number of a line within a source file",
    aliased_type = aftermath.types.builtin.uint64_t)

am_source_character_t = BaseTypeFixedWidthIntegerAlias(
    name = "am_source_character_t",
    entity = "source character position",
    comment = "Zero-index number of a character within a source line",
    aliased_type = aftermath.types.builtin.uint64_t)

am_string = BaseType(
    name = "am_string",
    entity = "string",
    comment = "Alias for a C string",
    aliased_type = aftermath.types.builtin.charp,
    format_string = "s",
    tags = [ Destructor("free", False) ])

aftermath.config.addBaseTypes(
    am_bool_t,
    am_timestamp_t,
    am_counter_value_t,
    am_event_collection_id_t,
    am_cpu_t,
    am_counter_t,
    am_nanoseconds_t,
    am_hierarchy_id_t,
    am_hierarchy_node_id_t,
    am_state_t,
    am_source_line_t,
    am_source_character_t,
    am_string)
