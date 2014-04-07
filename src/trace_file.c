/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "trace_file.h"
#include "ansi_extras.h"
#include "convert.h"
#include <stdio.h>

const char* worker_state_names[] = {
	"seeking",
	"taskexec",
	"tcreate",
	"resdep",
	"tdec",
	"bcast",
	"init",
	"estimate_costs",
	"reorder"
};

int trace_header_conversion_table[] = {
	FIELD_SIZE(struct trace_header, magic),
	FIELD_SIZE(struct trace_header, version),
	FIELD_SIZE(struct trace_header, day),
	FIELD_SIZE(struct trace_header, month),
	FIELD_SIZE(struct trace_header, year),
	FIELD_SIZE(struct trace_header, hour),
	FIELD_SIZE(struct trace_header, minute),
	CONVERSION_TABLE_END
};

#define EVENT_HEADER_CONVERSION_FIELDS \
	FIELD_SIZE(struct trace_event_header, type), \
	FIELD_SIZE(struct trace_event_header, time), \
	FIELD_SIZE(struct trace_event_header, cpu), \
	FIELD_SIZE(struct trace_event_header, worker), \
	FIELD_SIZE(struct trace_event_header, active_task), \
	FIELD_SIZE(struct trace_event_header, active_frame)

int trace_event_header_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	CONVERSION_TABLE_END
};

int trace_state_event_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct trace_state_event, end_time),
	FIELD_SIZE(struct trace_state_event, state),
	CONVERSION_TABLE_END
};

int trace_comm_event_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct trace_comm_event, type),
	FIELD_SIZE(struct trace_comm_event, src_or_dst_cpu),
	FIELD_SIZE(struct trace_comm_event, size),
	FIELD_SIZE(struct trace_comm_event, prod_ts),
	FIELD_SIZE(struct trace_comm_event, what),
	CONVERSION_TABLE_END
};

int trace_single_event_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct trace_single_event, type),
	FIELD_SIZE(struct trace_single_event, what),
	CONVERSION_TABLE_END
};

int trace_counter_description_conversion_table[] = {
	FIELD_SIZE(struct trace_counter_description, type),
	FIELD_SIZE(struct trace_counter_description, counter_id),
	FIELD_SIZE(struct trace_counter_description, name_len),
	CONVERSION_TABLE_END
};

int trace_counter_event_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct trace_counter_event, counter_id),
	FIELD_SIZE(struct trace_counter_event, value),
	CONVERSION_TABLE_END
};

int trace_frame_info_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct trace_frame_info, addr),
	FIELD_SIZE(struct trace_frame_info, numa_node),
	FIELD_SIZE(struct trace_frame_info, size),
	CONVERSION_TABLE_END
};

int trace_cpu_info_conversion_table[] = {
	EVENT_HEADER_CONVERSION_FIELDS,
	FIELD_SIZE(struct trace_cpu_info, numa_node),
	CONVERSION_TABLE_END
};

int trace_global_single_event_conversion_table[] = {
	FIELD_SIZE(struct trace_global_single_event, type),
	FIELD_SIZE(struct trace_global_single_event, time),
	FIELD_SIZE(struct trace_global_single_event, single_type),
	CONVERSION_TABLE_END
};

int trace_verify_header(struct trace_header* header)
{
	return (header->magic == TRACE_MAGIC &&
		trace_version_compatible(header->version) &&
		header->day > 0 && header->day <= 31 &&
		header->month > 0 && header->month <= 12 &&
		header->hour < 24 && header->minute < 60);
}
