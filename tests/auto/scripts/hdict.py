# Copyright (C) 2016 Andi Drebes <andi.drebes@lip6.fr>
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
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

from sets import Set

# Split s on dots. If s is empty, the empty list is returned.
def splitzero(s):
    ret = s.split(".")

    if ret[0] == '' and not ret[1:]:
        return []

    return ret

# Hierarchical dictionaries are dictionaries with multiple levels. For example,
# an instance of a hierarchical dictionary with values "foo", "bar" and "baz"
# for the respective paths "a.b", "a.b.c" and "d.e.f" has the following
# structure:
#
# {
#    "next_level" : {
#       "a" : {
#          "next_level" : {
#             "b" : {
#                "value" : "foo",
#                "next_level" : {
#                   "c" : {
#                      "value" : "bar"
#                   }
#                }
#             }
#          }
#       },
#       "d" : {
#          "next_level" : {
#             "e" : {
#                "next_level" : {
#                   "f" : {
#                      "value" : "baz"
#                   }
#                }
#             }
#          }
#       }
#    }
# }

# Put a value into the hierarchical dictionary. The path might either be a
# string (e.g., "a.b.c") or a list (e.g., ["a", "b", "c"])
def hdict_put_val(dct, path, val):
    if type(path) == str:
        path = splitzero(path)

    if not "next_level" in dct:
            dct["next_level"] = {}

    if not path[1:]:
        if not path[0] in dct["next_level"]:
            dct["next_level"][path[0]] = {"value" : val}
        else:
            dct["next_level"][path[0]]["value"] = val
    else:
        if not path[0] in dct["next_level"]:
            dct["next_level"][path[0]] = {"next_level" : {}}

        hdict_put_val(dct["next_level"][path[0]], path[1:], val)

# Check if a path exists in a hierarchical dictionary. The path might either be
# a string (e.g., "a.b.c") or a list (e.g., ["a", "b", "c"]). An existing path
# does not necessarily have an associated value. For example, there might only
# be a value at a.b.c, but a, a.b and a.b.c are all existing paths.
def hdict_has(dct, path):
    if type(path) == str:
        path = splitzero(path)

    if not path:
        return True

    if (not "next_level" in dct) or \
       (not path[0] in dct["next_level"]):
        return False
    else:
        return hdict_has(dct["next_level"][path[0]], path[1:])

# Check if a hierarchical dictionary contains a value for a specific path.
def hdict_has_val(dct, path):
    lvl = hdict_get_level(dct, path)

    if (not lvl) or (not "value" in lvl):
        return False

    return True

# Returns the path in list form of the first non-existing sub-path of the
# specified path. For example, if a.b.c exists, hdict_first_new(dct,
# "a.b.c.d.e") returns ["a", "b", "c", "d"]. If the path exists, the function
# returns an empty list.
def hdict_first_new(dct, path):
    if type(path) == str:
        path = splitzero(path)

    if not path:
        return []

    if (not "next_level" in dct) or \
       (not path[0] in dct["next_level"]):
        return [ path[0] ]
    else:
        r = hdict_first_new(dct["next_level"][path[0]], path[1:])
        if r:
            return [ path[0] ] + r
        else:
            return []

# Get the hierarchical sub-dictionary for a path
def hdict_get_level(dct, path):
    if type(path) == str:
        path = splitzero(path)

    if not path:
        return dct

    if not path[1:]:
        if (not "next_level" in dct) or \
           (not path[0] in dct["next_level"]):
            return None
        else:
            return dct["next_level"][path[0]]

    if (not "next_level" in dct) or \
       (not path[0] in dct["next_level"]):
        return None

    return hdict_get_level(dct["next_level"][path[0]], path[1:])

# Returns the value for a specific path. If the path does not exist or no value
# is associated, the function returns None.
def hdict_get_val(dct, path):
    lvl = hdict_get_level(dct, path)

    if (not lvl) or \
       not "value" in lvl:
        return None

    return lvl["value"]

# Associate the value val to a path if the path does not exist.
def hdict_put_val_if_not(dct, path, val):
    if not hdict_has_val(dct, path):
        hdict_put_val(dct, path, val)
        return val
    else:
        return hdict_get_val(dct, path)

# Return the value of a counter associated to a path. If the path does not
# exist, create a new counter and initialize it with 0.
def hdict_ctr_get(dct, path):
    return hdict_put_val_if_not(dct, path, 0)

# Locate the counter associated to a path, increment it and return its old value
def hdict_ctr_get_inc(dct, path):
    ctr = hdict_ctr_get(dct, path)
    hdict_put_val(dct, path, ctr+1)
    return ctr

# Checks if the value of a path refers to a set and if val ius contained in the
# set
def hdict_set_has(dct, path, val):
    return hdict_has(dct, path) and \
       val in hdict_get(dct, path)

# Add the value val to the set associated to a path. If the path does not exist
# or is not associated to a set, create the set.
def hdict_set_add(dct, path, val):
    s = hdict_put_val_if_not(dct, path, Set())
    s.add(val)

# Returns the list associated to a path. If the path is not associated to a
# list, create an empty list.
def hdict_list_get(dct, path):
    return hdict_put_val_if_not(dct, path, [])

# Add the value val to the list associated to a path. If the path does not refer
# to a list, create an empty list and add the value to it.
def hdict_list_append(dct, path, val):
    lst = hdict_put_val_if_not(dct, path, [])
    lst.append(val)

# Extend the list associated to a path with lst. If the path does not refer to a
# list, create an empty list and extend it.
def hdict_list_extend(dct, path, lst):
    l = hdict_put_val_if_not(dct, path, [])
    l.extend(lst)
