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

def RaiseAbstractFunctionException(name, inst):
    """Raises an exception indicating that the method `name` is not implemented for
    the instance `inst`."""

    raise Exception("Method '" + name + "' not implemented for class '" +
                    str(inst.__class__.__name__)+ "'")

def AbstractFunction(f):
    """Decorator declaring a function abstract, which makes the decorated
    function throw an exception if invoked."""

    return lambda *args, **kwargs: \
        RaiseAbstractFunctionException(f.__name__, args[0])

def enforce_type(inst, t):
    """Verifies that `inst` is of type `t`. The parameter `t` may also be a list,
    in which case the function verifies if inst is an instance of at least one of
    the specified types. If the condition is not met, an exception is thrown
    """

    if isinstance(t, list):
        for tt in t:
            if isinstance(inst, tt):
                return

        raise Exception("Expected a type from '" + \
                        str(list(map(lambda tt: tt.__name__, t))) + "', but given " + \
                        "type is '"+inst.__class__.__name__+"'")
    else:
        if not isinstance(inst, t):
            raise Exception("Expected type '" + t.__name__ + "', but given " + \
                            "type is '"+inst.__class__.__name__+"'")

def enforce_type_list(lst, t):
    """Verifies that `lst` is a list and all of its items are instances of `t`.
    The parameter `t` may also be a list, in which case the function verifies if
    each item in `lst` is an instance of at least one of the specified types.
    If the condition is not met, an exception is thrown.
    """

    enforce_type(lst, list)

    for inst in lst:
        enforce_type(inst, t)

def enforce_type_tuple(tpl, t):
    """Verifies that `tpl` is a tuple and all of its items are instances of `t`.
    The parameter `t` may also be a list, in which case the function verifies if
    each item in `tpl` is an instance of at least one of the specified types.
    If the condition is not met, an exception is thrown.
    """

    enforce_type(tpl, tuple)

    for inst in tpl:
        enforce_type(inst, t)

def enforce_type_dict_values(d, t):
    """Verifies that the values of the dictionary `d` are instances of `t`. The
    parameter `t` may also be a list, in which case the function verifies if
    each value is an instance of at least one of the specified types.  If the
    condition is not met, an exception is thrown.
    """

    for v in d.values():
        enforce_type(v, t)
