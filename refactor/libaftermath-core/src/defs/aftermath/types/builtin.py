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

# Builtin types of the C language

from aftermath.types import Type, \
    TypeList, \
    NonCompoundType, \
    IntegerType, \
    FixedWidthIntegerType
import aftermath.tags

class BuiltinType(Type):
    """A type that is a built-in type of the C language or its standard
    library"""

    def __init__(self, alias, *args, **kwargs):
        super(BuiltinType, self).__init__(name = alias, *args, **kwargs)
        self.__alias = alias

    def getCType(self):
        return self.__alias

class BuiltinNonCompoundType(BuiltinType, NonCompoundType):
    """Builtin type that is an alias of a non-compound built-in type"""

    def __init__(self, *args, **kwargs):
        super(BuiltinNonCompoundType, self).__init__(*args, **kwargs)

class BuiltinIntegerType(BuiltinNonCompoundType, IntegerType):
    """Builtin type that is an alias of a built-in integer type"""

    def __init__(self, alias, *args, **kwargs):
        IntegerType.__init__(self, *args, name = "", **kwargs)
        BuiltinNonCompoundType.__init__(self, *args, alias = alias, **kwargs)

class BuiltinFixedWidthIntegerType(BuiltinNonCompoundType,
                                   FixedWidthIntegerType):
    """Builtin type that is an alias of a built-in integer type with an
    implementation-independent width in bits"""

    def __init__(self, *args, **kwargs):
        FixedWidthIntegerType.__init__(self, *args, name = "", **kwargs)
        BuiltinNonCompoundType.__init__(self, *args,
                                        alias = self.getCType(),
                                        **kwargs)

    def getCType(self):
        ret = "u" if not self.isSigned() else ""
        ret += "int"+str(self.getNumBits())+"_t"

        return ret

_integers = {}

for signed in [True, False]:
    for num_bits in [8, 16, 32, 64]:
        i = BuiltinFixedWidthIntegerType(signed = signed,
                               num_bits = num_bits,
                               comment = None,
                               entity = str(num_bits)+" bit integer")
        _integers[i.getCType()] = i

int8_t = _integers["int8_t"]
int16_t = _integers["int16_t"]
int32_t = _integers["int32_t"]
int64_t = _integers["int64_t"]

uint8_t = _integers["uint8_t"]
uint16_t = _integers["uint16_t"]
uint32_t = _integers["uint32_t"]
uint64_t = _integers["uint64_t"]

char = BuiltinNonCompoundType(
    alias = "char",
    entity = "character",
    comment = None,
    format_string = "c",
    pformat_string = "s")

charp = BuiltinNonCompoundType(
    alias = "char*",
    entity = None,
    comment = None,
    format_string = "s",
    tags = [
        aftermath.tags.Destructor("free", takes_address = False),
        aftermath.tags.dsk.DumpStdoutFunction(
            function_name = "am_dsk_charp_dump_stdout"
        )])

void = BuiltinNonCompoundType(
    alias = "void",
    entity = None,
    comment = None)

int = BuiltinIntegerType(
    alias = "int",
    entity = "integer",
    signed = True,
    comment = None,
    format_string = "d")

size_t = BuiltinIntegerType(
    alias = "size_t",
    entity = "size",
    signed = False,
    comment = None,
    format_string = "zu")

all_types = TypeList(_integers.values() + [ char, charp, int, size_t ])

# Add read and write function tags to bultin integer types
for t in _integers.values():
    if not t.hasTag(aftermath.tags.dsk.ReadFunction):
        t.addTag(aftermath.tags.dsk.ReadFunction(
            function_name = "am_dsk_"+t.getName()+"_read"
        ))

    if not t.hasTag(aftermath.tags.dsk.WriteFunction):
        t.addTag(aftermath.tags.dsk.WriteFunction(
            function_name = "am_dsk_"+t.getName()+"_write"
        ))

    if not t.hasTag(aftermath.tags.dsk.WriteToBufferFunction):
        t.addTag(aftermath.tags.dsk.WriteToBufferFunction(
            function_name = "am_dsk_"+t.getName()+"_write_to_buffer"
        ))

# Add dump function tag to all types
for t in all_types:
    if not t.hasTag(aftermath.tags.dsk.DumpStdoutFunction):
        t.addTag(aftermath.tags.dsk.DumpStdoutFunction(
            function_name = "am_dsk_"+t.getName()+"_dump_stdout"
        ))
