#!/usr/bin/env python

# Author: Andi Drebes <andi.drebes@lip6.fr>
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

import am_type_aux
import am_base_types
import am_mem_types
import am_dsk_types

# This is a helper module that makes type definitions for base types, on-disk
# types and in-memory types as well as helper functions easily accessible from
# within templates.

def definitions():
    return {
        "base" : {
            "types" : am_base_types.types,
            "types_list" : am_type_aux.typedict_to_list(am_base_types.types),
            "find" : am_base_types.find
        },
        "dsk" : {
            "types" : am_dsk_types.types,
            "types_list" : am_type_aux.typedict_to_list(am_dsk_types.types),
            "find" : am_dsk_types.find,
            "find_same_mem" : am_dsk_types.find_same_mem
        },
        "mem" : {
            "types" : am_mem_types.types,
            "types_list" : am_type_aux.typedict_to_list(am_mem_types.types),
            "find" : am_mem_types.find,
            "find_same_dsk" : am_mem_types.find_same_dsk
        },
        "topological_sort" : am_type_aux.topological_sort,
        "find_field" : am_type_aux.find_field,
        "filter_list_hasattrs" : am_type_aux.filter_list_hasattrs,
        "filter_list_hasdefs" : am_type_aux.filter_list_hasdefs,
        "isinttype" : am_type_aux.isinttype,
        "istuple" : lambda x: isinstance(x, tuple)
    }
