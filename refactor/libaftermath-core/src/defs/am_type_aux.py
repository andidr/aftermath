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

import copy
import re

# Returns a deep copy of d, extended with values from "defaults" for each key
# that does not appear in d.
def dict_set_default(d, defaults):
    ret = copy.deepcopy(d)

    for defkey in defaults.keys():
        if not defkey in ret.keys():
            ret[defkey] = copy.deepcopy(defaults[defkey])

    return ret

# Returns a deep copy of the list of dictionaries ds, where each dictionary is
# extended with default valuesfrom the dictionary "defaults".
def dictlist_set_default(ds, defaults):
    l = []

    for d in ds:
        l.append(dict_set_default(d, defaults))

    return l

# Merges a list of dictionaries dicts into a single dictionary by successively
# updating the first dictionary with the remaining entries using
# dict_set_default.
def merge_dicts(*dicts):
    merged = {}

    for d in dicts:
        merged = dict_set_default(merged, d)

    return merged

# Returns the value for a key from the first dictionary in a list of
# dictionaries containing the key.
def dicts_find(key, dicts):
    for d in dicts:
        if key in d.keys():
            return d[key]

    raise Exception("Could not find key "+key)

# Converts a dictionary with type definitions into a list. The value of a key in
# the type dictionary is itself a dictionary representing the type
# definition. This dictionary is extended with a new key "type" whose value is
# the type name. The final list is composed of the extended dictionaries.
def typedict_to_list(d):
    l = []

    for tname in d.keys():
        l.append(merge_dicts(d[tname], {"name" : tname }))

    return l

# Returns a list of types that a type type definition depends on. Only direct
# dependences are reported, i.e., transitive dependences are omitted.
def type_dependencies(t, types):
    deps = []

    if t["compound"]:
        for field in t["fields"]:
            if field["type"] in types.keys() and field["type"] not in deps:
                deps.append(field["type"])

    return deps

# Returns the definition of a field of a compound type t.
def find_field(t, field_name):
    for f in t["fields"]:
        if f["name"] == field_name:
            return f

    raise Exception("Type "+t["name"]+" does not have a field "+field_name)

# Filters a list of types according to a dictionary. The function returns the
# list of types that have all key-value pairs specified in the dictionary d
def filter_list_attrs(types, d):
    ret = []

    for t in types:
        add = True

        for key in d.keys():
            if not key in t.keys() or t[key] != d[key]:
                add = False
                break

        if add:
            ret.append(t)

    return ret

# Filters a list of types according to list of keys. The function returns the
# list of types that have all keys specified in the list attrs
def filter_list_hasattrs(types, attrs):
    ret = []

    for t in types:
        add = True

        for attr in attrs:
            if not attr in t.keys():
                add = False
                break

        if add:
            ret.append(t)

    return ret

# The function returns the list of types in the list of type definitions "types"
# whose attribute "defs" contains all entries specified in the list "defs".
def filter_list_hasdefs(types, defs):
    ret = []

    for t in types:
        add = True

        for d in defs:
            if not d in t["defs"]:
                add = False
                break

        if add:
            ret.append(t)

    return ret

# Determines the dependencies for type declarations for a set of types and
# returns the types as a topologically sorted list.
def topological_sort(types):
    deps = {}
    done = False
    order = []

    for tname in types.keys():
        deps[tname] = type_dependencies(types[tname], types)

    size_last = len(deps.keys())

    while deps.keys():
        for tname in deps.keys():
            if not deps[tname]:
                order.append(tname)

                for tname2 in deps.keys():
                    if tname in deps[tname2]:
                        deps[tname2].remove(tname)

                del deps[tname]

        if len(deps.keys()) == size_last:
            raise Exception("Cyclic dependences detected")
        else:
            size_last = len(deps.keys())

    return order

# Returns True if the parameter tname is an integer type (e.g., uint64_t,
# int8_t, ...), otherwise False.
def isinttype(tname):
    if re.match("u?int[0-9]+_t", tname):
        return True
    else:
        return False

# Returns the number of decimal digits (including the sign) for an integer of n
# bits
def max_decimal_digits(n):
    return len(str(2**n))+1
