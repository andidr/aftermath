/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
 * Copyright (C) 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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
 *
 * Description:
 * This program demonstrates how to generate an Aftermath trace file containing
 * state events.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* Aftermath headers */
#include <trace_file.h>
#include <ansi_extras.h>
#include <convert.h>

const char* worker_state_names[] = {
	"A",
	"B",
};

enum worker_state {
	WORKER_STATE_A = 0,
	WORKER_STATE_B = 276876922,
	WORKER_STATE_C = 276876923
};

int worker_state_ids[] = {
	WORKER_STATE_A,
	WORKER_STATE_B,
	WORKER_STATE_C
};

int num_worker_states = 2;

int main(int argc, char** argv)
{
	/* Trace file name and handle */
	FILE* fp;
	const char* filename = "events.ost";

	/* Trace header */
	struct trace_header dsk_header;

	/* Data structure representing a single state event */
	struct trace_state_event dsk_se;

        /* Metadata for states */
        struct trace_state_description dsk_sd;

	/* Determine date and time for trace header */
	time_t t = time(NULL);
	struct tm* now = localtime(&t);

	/* Open trace file */
	if(!(fp = fopen(filename, "w+"))) {
		fprintf(stderr, "Could not open file %s.\n", filename);
		exit(1);
	}

	/* Initialize header */
	dsk_header.magic = TRACE_MAGIC;
	dsk_header.version = TRACE_VERSION;
	dsk_header.day = now->tm_mday;
	dsk_header.month = now->tm_mon+1;
	dsk_header.year = now->tm_year+1900;
	dsk_header.hour = now->tm_hour;
	dsk_header.minute = now->tm_min;

	/* Convert header to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_header, sizeof(dsk_header),
			     trace_header_conversion_table, 0);

	/* State description A*/
	int name_len = strlen(worker_state_names[0]);

	/* Initialize a state description */
	dsk_sd.type = EVENT_TYPE_STATE_DESCRIPTION;
	dsk_sd.name_len = name_len;
	dsk_sd.state_id = worker_state_ids[0];

	/* Convert state description to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_sd, sizeof(dsk_sd), trace_state_description_conversion_table, 0);

	/* The name of the state must be written right
	   after the state description */
	fwrite(worker_state_names[0], name_len, 1, fp);

	/* First state: seeking from 0 to 1000000 cycles */
	dsk_se.header.type = EVENT_TYPE_STATE;
	dsk_se.header.time = 0;
	dsk_se.header.cpu = 0;
	dsk_se.header.worker = 0;
	dsk_se.header.active_task = 0;
	dsk_se.header.active_frame = 0;
	dsk_se.end_time = 1000000;
	dsk_se.state = WORKER_STATE_A;

	/* Convert state event to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_se, sizeof(dsk_se),
			     trace_state_event_conversion_table, 0);

	/* Second state: task execution from 1000000 to 2000000 cycles */
	dsk_se.header.type = EVENT_TYPE_STATE;
	dsk_se.header.time = 1000000;
	dsk_se.header.cpu = 0;
	dsk_se.header.worker = 0;
	dsk_se.header.active_task = 0;
	dsk_se.header.active_frame = 0;
	dsk_se.end_time = 2000000;
	dsk_se.state = WORKER_STATE_B;

	/* Convert state event to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_se, sizeof(dsk_se),
			     trace_state_event_conversion_table, 0);

	/* Third state(unknown): task execution from 1000000 to 2000000 cycles */
	dsk_se.header.type = EVENT_TYPE_STATE;
	dsk_se.header.time = 2000000;
	dsk_se.header.cpu = 0;
	dsk_se.header.worker = 0;
	dsk_se.header.active_task = 0;
	dsk_se.header.active_frame = 0;
	dsk_se.end_time = 3000000;
	dsk_se.state = WORKER_STATE_C;

	/* Convert state event to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_se, sizeof(dsk_se),
			     trace_state_event_conversion_table, 0);

	/* State description B */
	name_len = strlen(worker_state_names[1]);

	/* Initialize a state description */
	dsk_sd.type = EVENT_TYPE_STATE_DESCRIPTION;
	dsk_sd.name_len = name_len;
	dsk_sd.state_id = worker_state_ids[1];

	/* Convert state description to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_sd, sizeof(dsk_sd), trace_state_description_conversion_table, 0);

	/* The name of the state must be written right
	   after the state description */
	fwrite(worker_state_names[1], name_len, 1, fp);

	/* Close trace file */
	fclose(fp);

	return 0;
}
