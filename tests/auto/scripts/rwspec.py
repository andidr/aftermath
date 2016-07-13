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

import sys
import shlex
import re
import os
import numpy as np
from hdict import *
from sets import Set
import rwspec_defs as rwsd

# Invoke model triggers for the current level of a hierarchical dictionary of
# triggers htrig for the i-th component of the path and the instance inst.
def invoke_model_triggers_rec(htrig, path, i, path_len, inst):
    l = []

    if i >= path_len:
        return []

    curr = path[i]

    if "next_level" in htrig:
        for trig in htrig["next_level"]:
            if trig == curr or trig == "*":
                if "value" in htrig["next_level"][trig]:
                    for stmt in htrig["next_level"][trig]["value"]:
                        l.append(stmt.format(**inst["vals"]))
                if "next_level" in htrig:
                    nllst = invoke_model_triggers_rec(htrig["next_level"][trig],
                                                path, i+1, path_len, inst)
                    l.extend(nllst)

    return l

# Invoke all model triggers of the hierarchical dictionary htrig for an instance
# that would be inserted into the hierarchical dictionary of instances hinsts
def invoke_model_triggers(htrig, hinsts, path, inst):
    min_new = hdict_first_new(hinsts, path)

    if min_new:
        min_htrig = hdict_get_level(htrig, min_new[:-1])
        if min_htrig:
            return invoke_model_triggers_rec(min_htrig, path,
                                       len(min_new)-1, len(path), inst)

    return []

# Make a hierarchical dicionary from a dictionary composed of path keys and
# associated values
def make_hierarchical(dct):
    hdct = {}
    for k in dct:
        hdict_put_val(hdct, k, dct[k])

    return hdct

class trace:
    def __init__(self, name):
        self.insts = []
        self.hinsts = {}

        self.model_stmts = []
        self.name = name

    # Add an instance to the trace and invoke all triggers
    def add_inst(self, inst):
        t = rwsd.types[inst["type"]]
        path = splitzero(t["hierarchy"].format(**inst["vals"]))

        self.model_stmts.extend(
            invoke_model_triggers(rwsd.model_triggers, self.hinsts, path, inst))

        self.model_stmts.extend(
            map(lambda stmt_tpl:
                stmt_tpl.format(**inst["vals"]), t["gen_model"]))

        self.insts.append(inst)
        hdict_list_append(self.hinsts, path, inst)

    # Return a string containing all statements to generate the trace
    def gen_model_code(self):
        return "\t"+"\n\t".join(self.model_stmts)

    # Generate code for all checks after loading the trace. Returns a list of
    # statements
    def gen_check_code_rec(self, hinst, triggers, path):
        ret = []

        if "next_level" in hinst:
            for lvl_name in hinst["next_level"]:
                if triggers and "next_level" in triggers:
                    if lvl_name in triggers["next_level"]:
                        idx = lvl_name
                    elif "*" in triggers["next_level"]:
                        idx = "*"
                    else:
                        idx = None

                    if idx:
                        if "value" in triggers["next_level"][idx]:
                            if "value" in hinst["next_level"][lvl_name]:
                                insts = hinst["next_level"][lvl_name]["value"]
                            else:
                                insts = []

                            gen_chk = triggers["next_level"][idx]["value"]
                            ret.extend(gen_chk(path + [lvl_name], insts))

                        ret.extend(self.gen_check_code_rec(hinst["next_level"][lvl_name],
                                                           triggers["next_level"][idx],
                                                           path + [lvl_name]))

        return ret

    # Return a string containing all statements to check the trace after loading
    def gen_check_code(self):
        stmts = self.gen_check_code_rec(self.hinsts, rwsd.check_triggers, [])
        return "\t"+"\n\t".join(stmts)

    # Returns a string defining a unit test generating, loading and checking the
    # trace
    def gen_test(self):
        model_code = self.gen_model_code()
        check_code = self.gen_check_code()

        s = rwsd.one_test_template.format(test_name = self.name, **locals())
        s = s.replace("\n\t\n", "\n\n")
        s = re.sub("\n{3,}", "\n\n", s)
        return s

# Parse command-line arguments and return a triple composed of the list of test
# names, input files and the output file
def parse_args():
    i = 1
    l = len(sys.argv)

    infiles = []
    outfile = []
    test_name = []

    while i < l:
        if sys.argv[i] == "-o" and i >= l-1:
                die("Please specify an argument for "+sys.argv[i]+"\n")

        if sys.argv[i] == "-h" or sys.argv[i] == "--help":
            die_usage()
        elif sys.argv[i] == "-o":
            outfile = sys.argv[i+1]
            i += 2
        elif sys.argv[i] != "" and sys.argv[i][0] != "-":
            infiles += [ sys.argv[i] ]
            i += 1
        else:
            die("Uknown argument "+sys.argv[i]+"\n")

    return (infiles, outfile)

# Convert a list of pairs [(a . b), (c . d), ...] into
# a dictionary { a : b, c : d}
def make_dict(pairlist):
    d = {}

    for p in pairlist:
        d[p[0]] = p[1]

    return d

# Convert a single value using a conversion function
def convert_one_field((value, conversion_fun)):
    return conversion_fun(value)

# Convert a list of tokens for a type t info a dictionary of key-value pairs
def convert_fields(t, tokens):
    field_names = [ x[0] for x in rwsd.types[t]["fields"] ]
    field_convfuncs = [ x[1] for x in rwsd.types[t]["fields"] ]

    return make_dict(zip(field_names,
                         map(convert_one_field,
                             zip(tokens, field_convfuncs))))

# Process one line of input, already tokenized
def process_inst(trace, tokens, filename, lineno):
    t = tokens[0]

    if t not in rwsd.types.keys():
        die(filename+":"+str(lineno)+": error: Unknown token \""+t+"\"\n")

    if len(tokens)-1 != len(rwsd.types[t]["fields"]):
        die(filename+":"+str(lineno)+": error: Expecting exactly "+\
            str(len(rwsd.types[t]["fields"]))+" tokens after "+t+": "+\
            (", ".join([ x[0] for x in rwsd.types[t]["fields"]]))+"\n")

    d = convert_fields(t, tokens[1:])
    inst = {"type" : t, "vals": d }
    trace.add_inst(inst)

# Print an error message and exit with error
def die(str):
    sys.stderr.write(str)
    sys.exit(1)

def die_usage():
    die("Usage: "+sys.argv[0]+" [-h] -o outfile infiles\n")

# Remove the suffix rem from a filename and convert the rest into a test name
def file2name(filename, rem = ""):
    if rem and filename.endswith(rem):
        return file2name(filename[0:-len(rem)], "")

    return re.sub(r"[^a-zA-Z0-9]", "_", filename)

# Tokenize one line of input
def tokenize(s):
    return map(lambda x: x, shlex.shlex(s, posix = True))

# Process one input file
def process_infile(infile):
    fpin = open(infile, "r")

    trcs = []
    curr_trace = None

    for lineno,line in enumerate(map(lambda x: x.strip(), fpin.readlines())):
        if line and line[0] != '#':
            tokens = tokenize(line)

            if tokens[0] == "@":
                if not tokens[1:]:
                    die(infile+":"+str(lineno+1)+": Missing directive\n")
                if tokens[1] != "TEST":
                    die(infile+":"+str(lineno+1)+": Unknown directive @"+tokens[1]+"\n")
                if not tokens[2:]:
                    die(infile+":"+str(lineno+1)+": Missing name\n")
                if tokens[3:]:
                    die(infile+":"+str(lineno+1)+": Unknown trailing token "+tokens[3]+"\n")

                name = tokens[2]

                basename = file2name(os.path.basename(infile), ".rwspec")
                curr_trace = trace(basename+"_"+name)
                trcs.append(curr_trace)
            else:
                if not curr_trace:
                    die(infile+":"+str(lineno+1)+": Instance before @TEST directive")
                else:
                    process_inst(curr_trace, tokens, infile, lineno+1)

    fpin.close()

    return trcs

if __name__ == "__main__":
    for t in rwsd.types:
        rwsd.types[t]["fields_dict"] = make_dict(rwsd.types[t]["fields"])

    rwsd.model_triggers = make_hierarchical(rwsd.model_triggers)
    rwsd.check_triggers = make_hierarchical(rwsd.check_triggers)

    infiles, outfile = parse_args()

    if not infiles:
        die("No input files specified.\n")

    fpout = open(outfile, "w+")
    tests = ""

    # Process all input files
    test_names = []

    for infile in infiles:
        for trc in process_infile(infile):
            tests += trc.gen_test()
            test_names.append(trc.name)

    # Generate test suite
    test_suite_def = "UNIT_TEST_SUITE("+\
                     file2name(os.path.basename(outfile), ".c")+")\n{\n"

    for name in test_names:
        test_suite_def += "\tADD_TEST("+name+");\n"

    test_suite_def += "}\nEND_TEST_SUITE()\n"

    fpout.write(rwsd.file_template.format(tests = tests,
                                          test_suite_def = test_suite_def))
