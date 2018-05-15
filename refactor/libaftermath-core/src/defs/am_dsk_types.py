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

# Definition of in-memory data structures. See doc/type_definitions for
# documentation.

__default_defs = ["type", "dsk_read", "dsk_write"]

__default_fields = {
    "compound" : True,
    "destructor" : None,
    "needs_constructor" : False,
    "defs" : __default_defs,
    "is_frame" : True
}

# Header at the beginning of a trace file
header = {
    "am_dsk_header" : {
        "is_frame" : False,
        "fields" : [
            {"name" : "magic",
             "type" : "uint32_t",
             "comment" : "Magic number for Aftermath trace files"},
            {"name" : "version",
             "type" : "uint32_t",
             "comment" : "Version of the trace format" }
        ]
    }
}

# Header of each frame
type_fields = [ { "name" : "type",
                  "type" : "uint32_t",
                  "comment" : "Type of this frame"} ]

# Header for each frame that defines something global for a trace
global_frame_fields = type_fields

# Header for each frame that defines an event associated to an event collection
event_frame_fields = type_fields + [
    { "name" : "collection_id",
      "type" : "uint32_t",
      "comment" : "ID of the event collection this event belongs to"} ]

frames = {
    "am_dsk_frame_type_id" : {
        "assert" : False,
        "entity" : "frame type ID association",
        "fields" : global_frame_fields + [
            {"name" : "id",
             "type" : "uint32_t",
             "comment" : "Numerical ID that will be associated with the type"},
            {"name" : "type_name",
             "type" : "am_dsk_string",
             "comment" : "Frame type as a string" }]
    },

    "am_dsk_hierarchy_description" : {
        "assert" : False,
        "entity" : "hierarchy description",
        "fields" : global_frame_fields + [
            {"name" : "id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of this hierarchy"},
            {"name" : "name",
             "type" : "am_dsk_string",
             "comment" : "Name of the hierarchy" }]
    },

    "am_dsk_hierarchy_node" : {
        "assert" : False,
        "entity" : "hierarchy node",
        "fields" : global_frame_fields + [
            {"name" : "hierarchy_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the hierarchy this node belongs to"},
            {"name" : "id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of this hierarchy node"},
            {"name" : "parent_id", # Special value: 0 -> Does not have a parent
             "type" : "uint32_t",
             "comment" : "Numerical ID of the parent of this node"},
            {"name" : "name",
             "type" : "am_dsk_string",
             "comment" : "Name of this node" }]
    },

    "am_dsk_state_description" : {
        "assert" : False,
        "entity" : "state description",
        "defs" : ["dsk_to_mem_copy_function"] + __default_defs,
        "fields" : global_frame_fields + [
            {"name" : "state_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the state"},
            {"name" : "name",
             "type" : "am_dsk_string",
             "comment" : "Name of the state" }],
        "process" : [
            {"type" : "per_trace_array",
             "args" : {
                "trace_array_field" : "state_descriptions",
                "trace_array_struct_name" : "am_state_description_array",
                "mem_struct_name" : "am_state_description",
                "dsk_struct_sort_field" : "state_id",
                "dsk_to_mem_function" : "am_dsk_state_description_to_mem"
             }
            }
        ],
        "to_mem_copy_fields" : ["state_id", "name"]
    },

    "am_dsk_state_event" : {
        "assert" : True,
        "defs" : ["dsk_to_mem_copy_function"] + __default_defs,
        "entity" : "state event",
        "fields" : event_frame_fields + [
            {"name" : "state",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the state"},
            {"name" : "interval",
             "type" : "am_dsk_interval",
             "comment" : "Interval during which the state was active" }],
        "process" : [
            {"type" : "per_event_collection_interval",
             "args" : {
                 "ecoll_array_type_name" : "am::generic::state",
                 "ecoll_array_struct_name" : "am_state_event_array",
                 "mem_struct_name" : "am_state_event",
                 "mem_struct_interval_field" : "interval",
                 "dsk_struct_interval_field" : "interval",
                 "dsk_struct_ecoll_id_field" : "collection_id",
                 "dsk_to_mem_function" : "am_dsk_state_event_to_mem"
             }
            }
        ],
        "to_mem_copy_fields" : [("state", "state_idx"), "interval"],
        "timestamp_min_max_update" : {
            "type" : "interval",
            "field" : "interval"
        }
    },

    "am_dsk_counter_description" : {
        "assert" : False,
        "entity" : "counter description",
        "defs" : ["dsk_to_mem_copy_function"] + __default_defs,
        "fields" : global_frame_fields + [
            {"name" : "counter_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the counter"},
            {"name" : "name",
             "type" : "am_dsk_string",
             "comment" : "Name of the counter" }],
        "process" : [
            {"type" : "per_trace_array",
             "args" : {
                 "trace_array_field" : "counter_descriptions",
                 "trace_array_struct_name" : "am_counter_description_array",
                 "mem_struct_name" : "am_counter_description",
                 "dsk_struct_sort_field" : "counter_id",
                 "dsk_to_mem_function" : "am_dsk_counter_description_to_mem"
             }
            }
        ],
        "to_mem_copy_fields" : ["counter_id", "name"]
    },

    "am_dsk_counter_event" : {
        "assert" : False,
        "entity" : "counter sample",
        "defs" : ["dsk_to_mem_copy_function"] + __default_defs,
        "fields" : event_frame_fields + [
            {"name" : "counter_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the counter this sample is for"},
            {"name" : "time",
             "type" : "uint64_t",
             "comment" : "Timestamp of the sample" },
            {"name" : "value",
             "type" : "int64_t",
             "comment" : "Value of the counter sample" }],
        "process" : [
            {"type" : "per_event_collection_per_id_timestamp",
             "args" : {
                 "ecoll_array_type_name" : "am::generic::counter",
                 "ecoll_array_struct_name" : "am_counter_event_array_collection",
                 "id_array_struct_name" : "am_counter_event_array",
                 "dsk_struct_ecoll_id_field" : "collection_id",
                 "dsk_struct_id_field" : "counter_id",
                 "dsk_struct_timestamp_field" : "time",
                 "mem_struct_name" : "am_counter_event",
                 "mem_struct_timestamp_field" : "time",
                 "dsk_to_mem_function" : "am_dsk_counter_event_to_mem"
             }
            }
        ],
        "to_mem_copy_fields" : ["time", "value"],
        "timestamp_min_max_update" : {
            "type" : "discrete",
            "field" : "time"
        }
    },

    "am_dsk_event_collection" : {
        "assert" : False,
        "entity" : "event collection",
        "fields" : global_frame_fields + [
            {"name" : "id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of this event collection"},
            {"name" : "name",
             "type" : "am_dsk_string",
             "comment" : "Name of this event collection"}]
    },

    "am_dsk_measurement_interval" : {
        "assert" : False,
        "entity" : "measurement interval",
        "defs" : ["dsk_to_mem_copy_function"] + __default_defs,
        "fields" : global_frame_fields + [
            {"name" : "interval",
             "type" : "am_dsk_interval",
             "comment" : "Start and end of the measurement interval"}],
        "process" : [
            {"type" : "per_trace_array",
             "args" : {
                 "trace_array_field" : "measurement_intervals",
                 "trace_array_struct_name" : "am_measurement_interval_array",
                 "mem_struct_name" : "am_measurement_interval",
                 "dsk_struct_sort_field" : "interval.start",
                 "dsk_to_mem_function" : "am_dsk_measurement_interval_to_mem"
             }
            }
        ],
        "to_mem_copy_fields" : ["interval"],
        "timestamp_min_max_update" : {
            "type" : "interval",
            "field" : "interval"
        }
    },

    "am_dsk_event_mapping" : {
        "assert" : False,
        "entity" : "event mapping",
        "fields" : global_frame_fields + [
            {"name" : "collection_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of this event collection"},

            {"name" : "hierarchy_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the hierarchy for the mapping"},

            {"name" : "node_id",
             "type" : "uint32_t",
             "comment" : "Numerical ID of the hierarchy node for the mapping"},

            {"name" : "interval",
             "type" : "am_dsk_interval",
             "comment" : "Interval during which the event collection was associated to the node" }
        ]
    }
}

aux_types = {
    "am_dsk_string" : {
        "needs_constructor" : True,
        "is_frame" : False,
        "entity" : "string",
        "defs" : ["destructor", "type"],
        "fields" : [
            {"name" : "len",
             "type" : "uint32_t",
             "comment" : "Length of the string"},
            {"name" : "str",
             "type" : "char*",
             "comment" : "Characters of the string (not zero-terminated)"}
        ]
    },

    "am_dsk_interval" : {
        "comment" : "An interval with a start and end timestamp.",
        "is_frame" : False,
        "defs" : ["dsk_to_mem_copy_function"] + __default_defs,
        "entity" : "interval",
        "fields" : [
            {"name" : "start",
             "type" : "uint64_t",
             "comment" : "Start of the interval"},
            {"name" : "end",
             "type" : "uint64_t",
             "comment" : "End of the interval"}
        ],
        "to_mem_copy_fields" : ["start", "end"]
    }
}

def __expand_types(types):
    ret = {}

    for tname in types.keys():
        t = types[tname]
        t = am_type_aux.dict_set_default(t, __default_fields)
        types[tname] = t
        t["name"] = tname

    for tname in am_type_aux.topological_sort(types):
        t = types[tname]

        if t["compound"]:
            if not("c_type" in t.keys()):
                t["c_type"] = "struct "+tname

            for field in t["fields"]:
                field_type = am_type_aux.dicts_find(field["type"], [types, am_base_types.types])

                if field_type["destructor"]:
                    if not "destructor" in t["defs"]:
                        t["defs"].append("destructor")

                    t["destructor"] = tname+"_destroy"
                    break

        ret[tname] = t

    return ret

def find(tname):
    return am_type_aux.dicts_find(tname, [types, am_base_types.types])

def find_same_mem(mem_name):
    # Integer types have the same name, without prefix
    if am_type_aux.isinttype(mem_name):
        return find(mem_name)

    if not mem_name.startswith("am_"):
        raise Exception("Unexpected type name "+mem_name+". Expected type starting with am_")

    return find("am_dsk_"+mem_name[3:])

types = __expand_types(am_type_aux.merge_dicts(header, frames, aux_types))
