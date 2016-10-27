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

import numpy as np
from rwspec_helpers import *

types = {
    "MEASUREMENT_START" : {
        "fields" : [("time", np.uint64)],
        "gen_model" : [ "ASSERT_EQUALS(am_trace_start_measurement_interval(&trace, {time}), 0);" ],
        "hierarchy" : "mes.global_single_events"
    },

    "MEASUREMENT_END" : {
        "fields" : [("time", np.uint64)],
        "gen_model" : [ "ASSERT_EQUALS(am_trace_end_measurement_interval(&trace, {time}), 0);" ],
        "hierarchy" : "mes.global_single_events"
    },

    "STATE_DESCRIPTION" : {
        "fields" : [("state_id", np.uint32),
                    ("name", str)
        ],
        "gen_model" : [ "ASSERT_EQUALS(am_trace_register_state(&trace, {state_id}, \"{name}\"), 0);" ],
        "hierarchy" : "mes.states"
    },

    "STATE_EVENT" : {
        "fields" : [("cpu", np.uint32),
                    ("state_id", np.uint32),
                    ("start", np.uint64),
                    ("end", np.uint64)
        ],
        "gen_model" : [ "ASSERT_EQUALS(am_event_set_trace_state(am_es{cpu}, {state_id}, {start}, {end}), 0);" ],
        "hierarchy" : "mes.cpu.{cpu}.state_events"
    },

    "COUNTER_DESCRIPTION" : {
        "fields" : [("counter_id", np.uint32),
                    ("name", str)
        ],
        "gen_model" : [ "ASSERT_EQUALS(am_trace_register_counter(&trace, {counter_id}, \"{name}\"), 0);" ],
        "hierarchy" : "mes.counters"
    },

    "COUNTER_EVENT" : {
        "fields" : [("cpu", np.uint32),
                    ("counter", np.uint32),
                    ("time", np.uint64),
                    ("value", np.uint64)
        ],
        "gen_model" : [ "ASSERT_EQUALS(am_event_set_trace_counter(am_es{cpu}, {counter}, {time}, {value}), 0);" ],
        "hierarchy" : "mes.cpu.{cpu}.counter.{counter}"
    }
}

model_triggers = {
    "mes.cpu.*" : [
        "",
        "ASSERT_EQUALS(am_trace_register_cpu(&trace, {cpu}, DEFAULT_DATA_SIZE), 0);",
        "struct am_event_set* am_es{cpu} = am_trace_get_event_set(&trace, {cpu});",
        "ASSERT_NONNULL(am_es{cpu});",
        ""]
}

check_triggers = {
    "mes.global_single_events" :
        lambda path, insts:
                check_array(insts, "mes.global_single_events", \
                            lambda inst, elem:
                                check_fields(inst, elem, []) + \
                                [ "ASSERT_EQUALS("+elem+"type, "+\
                                  ("GLOBAL_SINGLE_TYPE_MEASURE_START" \
                                   if inst["type"] == "MEASUREMENT_START" \
                                   else "GLOBAL_SINGLE_TYPE_MEASURE_END")+");"
                              ]),

    "mes.states" :
            lambda path, insts:
                check_fields_array(insts, "mes.states", []),

    "mes.counters" :
            lambda path, insts:
                check_fields_array(insts, "mes.counters", []),

    "mes.cpu.*.state_events" :
        lambda path, insts:
                check_fields_array(insts, "es{cpu}->state_events", ["cpu"]),

    "mes.cpu.*" :
        lambda path, insts:
            format_all([
                "",
                "struct event_set* es{cpu} = multi_event_set_find_cpu(&mes, {cpu});",
                "ASSERT_NONNULL(am_es{cpu});",
                ""], cpu = path[2]),

    "mes.cpu.*.counter.*" :
        lambda path, insts:
            format_all(
                ["struct counter_description* es{cpu}_cdesc{counter} = multi_event_set_find_counter_description(&mes, {counter});",
                 "ASSERT_NONNULL(es{cpu}_cdesc{counter});",
                 "int es{cpu}_ctres{counter}_idx = event_set_get_counter_event_set(es{cpu}, es{cpu}_cdesc{counter}->index);",
                 "ASSERT_DIFFERENT(es{cpu}_ctres{counter}_idx, -1);",
                 "struct counter_event_set* es{cpu}_ctres{counter};",
                 "es{cpu}_ctres{counter} = &es{cpu}->counter_event_sets[es{cpu}_ctres{counter}_idx];",
                 ""],
                cpu = path[2],
                counter = path[4]) +\
            check_fields_array(insts, "es{cpu}_ctres{counter}->events", ["counter", "cpu"]),
}

one_test_template = \
"""UNIT_TEST({test_name})
{{
	struct am_trace trace;
	struct multi_event_set mes;
	FILE* fp;
	char tmpname[] = \"test-tmp/{test_name}XXXXXX\";
	off_t bytes_read;

	ASSERT_EQUALS(am_trace_init(&trace, DEFAULT_DATA_SIZE), 0);
	fp = tmpfile_template(tmpname, \"wb+\");
	ASSERT_NONNULL(fp);

{model_code}

	ASSERT_EQUALS(am_trace_dump_fp(&trace, fp), 0);
	ASSERT_EQUALS(fclose(fp), 0);
	am_trace_destroy(&trace);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);

{check_code}

	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}}
END_TEST()

"""

file_template = \
"""#include <unit_tests.h>
#include <trace/trace.h>
#include <multi_event_set.h>
#include "tests/common.h"

#define DEFAULT_DATA_SIZE (1 << 20)

{tests}
{test_suite_def}

"""
