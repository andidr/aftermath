/**
 * Copyright (C) 2014 Andi Drebes <andi.drebes@lip6.fr>
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
 * counter events (e.g. hardware performance counter values).
 */

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

/* Aftermath headers */
#include <trace_file.h>
#include <ansi_extras.h>
#include <convert.h>

int main(int argc, char** argv)
{
	/* Trace file name and handle */
	FILE* fp;
	const char* filename = "events.ost";

	/* Trace header */
	struct trace_header dsk_header;

	/* Metadata for the counter */
	struct trace_counter_description dsk_cd;
	const char* counter_name ="sine-test";

	/* Data structure representing a single counter sample */
	struct trace_counter_event dsk_cre;

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

	/* Initialize counter description */
	dsk_cd.type = EVENT_TYPE_COUNTER_DESCRIPTION;
	dsk_cd.name_len = strlen(counter_name);
	dsk_cd.counter_id = 0;

	/* Convert counter description to on-disk endianness and write to disk */
	write_struct_convert(fp, &dsk_cd, sizeof(dsk_cd),
			     trace_counter_description_conversion_table, 0);

	/* The name of the counter must be written right
	   after the counter description */
	fwrite(counter_name, strlen(counter_name), 1, fp);

	/* Generate some counter samples */
	for(int i = 0; i < 1024; i++) {
		/* Initialize event header */
		dsk_cre.header.type = EVENT_TYPE_COUNTER;
		dsk_cre.header.time = i*1000000;
		dsk_cre.header.cpu = 0;
		dsk_cre.header.worker = 0;
		dsk_cre.header.active_task = 0;
		dsk_cre.header.active_frame = 0;

		/* Must be the ID from the counter description */
		dsk_cre.counter_id = dsk_cd.counter_id;

		/* Absolute 64-bit value of the sample */
		dsk_cre.value = 1000.0*(sin(i)+1.0);

		/* Convert counter event to on-disk endianness and write to disk */
		write_struct_convert(fp, &dsk_cre, sizeof(dsk_cre),
				     trace_counter_event_conversion_table, 0);
	}

	/* Close trace file */
	fclose(fp);

	return 0;
}
