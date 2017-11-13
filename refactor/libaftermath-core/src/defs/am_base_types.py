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

# Definition of in-memory data structures. See doc/type_definitions for
# documentation.

__default_fields = {
    "compound" : False,
    "destructor" : None,
    "needs_constructor" : False,
    "defs" : ["type"]
}

types = {
    "am_cpu_t" : {
        "comment" : "Numerical ID of a CPU",
        "c_def" : "uint32_t"
    },

    "am_state_t" : {
        "comment" : "Numerical ID of a state",
        "c_def" : "uint32_t"
    },

    "am_counter_t" : {
        "comment" : "Numerical ID of a counter",
        "c_def" : "uint32_t"
    },

    "am_counter_value_t" : {
        "comment" : "Type for counter samples",
        "c_def" : "uint64_t"
    },

    "am_timestamp_t" : {
        "comment" : "Time stamp expressed in cycles",
        "c_def" : "uint64_t"
    },

    "am_nanoseconds_t" : {
        "comment" : "Time stamp expressed in nanoseconds",
        "c_def" : "uint64_t"
    },

    "am_hierarchy_id_t" : {
        "comment" : "Numerical ID of a hierarchy",
        "c_def" : "uint32_t"
    },

    "am_hierarchy_node_id_t" : {
        "comment" : "Numerical ID of a hierarchy node",
        "c_def" : "uint32_t"
    },

    "am_event_collection_id_t" : {
        "comment" : "Numerical ID of an event collection",
        "c_def" : "uint32_t"
    },

    "char*" : {"c_type" : "char*",
               "defs" : [],
               "destructor" : "am_free_charp"}
}

def __expand_types(types):
    # Add integer types from the standard library
    for b in [8, 16, 32, 64]:
        for s in ["u", ""]:
            intname = s+"int"+str(b)+"_t"

            if not intname in types.keys():
                types[intname] = {
                    "c_def" : intname,
                    "defs" : []
                }

    # Expand types whose name is identical with the name of the C type
    for tname in types.keys():
        types[tname]["name"] = tname

        types[tname] = am_type_aux.dict_set_default(types[tname], \
                                                    __default_fields)

        if "c_type" not in types[tname].keys():
            types[tname]["c_type"] = tname

    return types

types = __expand_types(types)

def find(tname):
    return am_type_aux.dicts_find(tname, [types])
