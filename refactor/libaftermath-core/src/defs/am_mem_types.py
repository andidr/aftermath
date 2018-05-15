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
import copy

# Definition of in-memory data structures. See doc/type_definitions for
# documentation.

__default_fields = {
    "compound" : True,
    "destructor" : None,
    "needs_constructor" : False,
    "is_pointer" : False,
    "defs" : ["type"]
}

__pointer_clear_fields = ["defs", "postprocess" ]

types = {
    "am_string" : {
        "compound" : False,
        "entity" : "string",
        "defs" : [],
        "c_type" : "char*",
        "destructor" : "free",
        "needs_constructor" : True,
        "is_pointer" : True
    },

    "am_bool" : {
        "compound" : False,
        "entity" : "boolean",
        "defs" : [],
        "c_type" : "int"
    },

    "am_source_location" : {
        "entity" : "source location",
        "fields" : [
            {"name" : "file",
             "type" : "am_string",
             "comment" : "File containing the source"},
            {"name" : "line",
             "type" : "am_source_line_t",
             "comment" : "Zero-indexed number of the line within the file"},
            {"name" : "character",
             "type" : "am_source_character_t",
             "comment" : "Zero-indexed number of the character within the line"}
        ]
    },

    "am_time_offset" : {
        "comment" : "Difference between two timestamps",
        "entity" : "timestamp difference",
        "fields" : [
            {"name" : "abs",
             "type" : "am_timestamp_t",
             "comment" : "Absolute difference"},
            {"name" : "sign",
             "type" : "am_bool",
             "comment" : "If sign != 0 the difference is negative"}
        ]
    },

    "am_interval" : {
        "comment" : "An interval with a start and end timestamp.",
        "entity" : "interval",
        "fields" : [
            {"name" : "start",
             "type" : "am_timestamp_t",
             "comment" : "Start of the interval"},
            {"name" : "end",
             "type" : "am_timestamp_t",
             "comment" : "End of the interval"}
        ]
    },

    "am_state_description": {
        "comment" : "A description associating a name with a numerical state ID",
        "entity" : "state description",
        "fields" : [
            {"name" : "state_id",
             "type" : "am_state_t",
             "comment" : "ID of the state this description is for"},
            {"name" : "name",
             "type" : "am_string",
             "comment" : "Name of the state"}]},

    "am_counter_description" : {
        "comment" : "A description associating a name with a numerical counter ID",
        "entity" : "counter description",
        "fields" : [
            {"name" : "counter_id",
             "type" : "am_counter_t",
             "comment" : "ID of the counter this description is for"},
            {"name" : "name",
             "type" : "am_string",
             "comment" : "Name of the counter"}]},

    "am_state_event": {
        "comment" : "A state (e.g., a run-time state)",
        "entity" : "state",
        "fields" : [
            {"name" : "interval",
             "type" : "am_interval",
             "comment" : "Interval during which the state was active"},
            {"name" : "state_idx",
             "type" : "am_state_t",
             "comment" : "ID of the state"},
        ],
        "postprocess" : [
            {"type" : "id_to_index",
             "args" : {
                 "element" : {
                     "type_name" : "am_state_event",
                     "field" : "state_idx",
                     "field_fmt" : "AM_STATE_T_FMT",
                     "field_entity" : "id",
                     "ecoll_array_type_name" : "am::generic::state"
                 },
                 "description" : {
                     "type_name" : "am_state_description",
                     "field" : "state_id",
                     "entity" : "state description",
                     "trace_array_field" : "state_descriptions"
                 }
             }
            }
        ]
    },

    "am_counter_event": {
        "comment" : "A counter event",
        "entity" : "state",
        "fields" : [
            {"name" : "time",
             "type" : "am_timestamp_t",
             "comment" : "Timestamp of the counter interval"},
            {"name" : "value",
             "type" : "am_counter_value_t",
             "comment" : "Value of the counter event"}
        ]
    },

    "am_measurement_interval": {
        "comment" : "A measurment interval",
        "entity" : "measurement interval",
        "fields" : [
            {"name" : "interval",
             "type" : "am_interval",
             "comment" : "Start and end of the measurement interval"}
        ]
    },

    "am_openstream_task_type": {
        "comment" : "An OpenStream task type",
        "entity" : "OpenStream task type",
        "fields" : [
            {"name" : "type_id",
             "type" : "uint64_t",
             "comment" : "Numerical ID of this task type"},

            {"name" : "name",
             "type" : "am_string",
             "comment" : "Name of this task (e.g., symbol in the executable)"},

            {"name" : "source",
             "type" : "am_source_location",
             "comment" : "Location of the source code for this task type"}
        ]
    }
}

def __expand_types(types):
    ret = {}

    for tname in types.keys():
        t = types[tname]
        t = am_type_aux.dict_set_default(t, __default_fields)
        t["basename"] = tname
        types[tname] = t

    ptrtypes = {}

    # Create a pointer type for each type
    for tname in types.keys():
        t = types[tname]
        ptrt = copy.deepcopy(t)
        ptrt["is_pointer"] = True
        ptrt["basename"] = tname

        for field in __pointer_clear_fields:
            ptrt.pop(field, None)

        ptrt["defs"] = []

        ptrtypes[tname+"*"] = ptrt

    types = am_type_aux.merge_dicts(types, ptrtypes)

    for tname in types.keys():
        t = types[tname]
        t["name"] = tname

        if t["compound"]:
            if not("c_type" in t.keys()):
                t["c_type"] = "struct "+tname

            for field in t["fields"]:
                fieldtype = find_in_sets(field["type"], \
                                         [am_base_types.types, types])

                if fieldtype["destructor"]:
                    if not "destructor" in t["defs"]:
                        t["defs"].append("destructor")

                    t["destructor"] = t["basename"]+"_destroy"
                    break

        ret[tname] = t

    return ret

def find_in_sets(tname, sets):
    return am_type_aux.dicts_find(tname, sets)

def find(tname):
    return find_in_sets(tname, [types, am_base_types.types])

def find_same_dsk(dsk_name):
    # Integer types have the same name, without prefix
    if am_type_aux.isinttype(dsk_name):
        return find(dsk_name)

    if not dsk_name.startswith("am_dsk_"):
        raise Exception("Unexpected type name "+dsk_name+". Expected type starting with am_dsk_")

    return find("am_"+dsk_name[7:])

types = __expand_types(types)
