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

import sys

class Configuration(object):
    """Central configuration for an Aftermath type system, collecting all types
    (base types, on-disk types, in-memory types, meta types)."""

    def __init__(self):
        from aftermath.types import TypeList

        self.__base_types = TypeList()
        self.__meta_types = TypeList()
        self.__on_disk_types = TypeList()
        self.__in_memory_types = TypeList()

    def getBaseTypes(self):
        return self.__base_types

    def addBaseType(self, t):
        self.__base_types.addType(t)

    def addBaseTypes(self, *ts):
        self.__base_types.addTypes(*ts)

    def getDskTypes(self):
        return self.__on_disk_types

    def addDskType(self, t):
        self.__on_disk_types.addType(t)

    def addDskTypes(self, *ts):
        self.__on_disk_types.addTypes(*ts)

    def getMemTypes(self):
        return self.__in_memory_types

    def addMemType(self, t):
        self.__in_memory_types.addType(t)

    def addMemTypes(self, *ts):
        self.__in_memory_types.addTypes(*ts)

    def getMetaTypes(self):
        return self.__meta_types

    def addMetaType(self, t):
        self.__meta_types.addType(t)

    def addMetaTypes(self, *ts):
        self.__meta_types.addTypes(*ts)

    def finalize(self):
        all_lists = [ self.__base_types,
                      self.__meta_types,
                      self.__on_disk_types,
                      self.__in_memory_types ]

        for tl in all_lists:
            for t in tl:
                t.finalize()

# Singleton instance for *the* Aftermath type system
# Add types to this instance to make them available in the type system
config = Configuration()
