#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import rwspec_defs as rwsd

# Return a list of statements with an assertion for each field of the instance,
# except those listed in filtered_fields. The expression for the instance in the
# generated code is defined by base.
def check_fields(inst, base, filtered_fields):
    ret = []

    base = base.format(**inst["vals"])

    for fname,ftype in rwsd.types[inst["type"]]["fields"]:
        if not fname in filtered_fields:
            val = inst["vals"][fname]
            if ftype == str:
                ret.append("ASSERT_EQUALS_STRING({base}{fname}, \"{val}\");".format(**locals()))
            else:
                ret.append("ASSERT_EQUALS({base}{fname}, {val});".format(**locals()))

    return ret

# Generate a list of statements checking every field for every instance in
# insts, except the fields listed in filtered_fields
def check_fields_array(insts, array_base, filtered_fields):
    ret = []

    for i,inst in enumerate(insts):
        ret.extend(check_fields(inst, array_base+"["+str(i)+"].", filtered_fields))

    return ret

# Generate a list of statements checking every instance in insts by calling
# check_one_inst_fun for each instance
def check_array(insts, array_base, check_one_inst_fun):
    ret = []

    for i,inst in enumerate(insts):
        ret.extend(check_one_inst_fun(inst, array_base+"["+str(i)+"]."))

    return ret

# Apply format() to all strings in lst
def format_all(lst, **arg_dct):
    return map(lambda s: s.format(**arg_dct), lst)
