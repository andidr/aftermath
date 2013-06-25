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

#include "paraver.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int read_paraver_samples(struct multi_event_set* mes, const char* file)
{
	FILE* fp;
	int res = 1;
	int day;
	int month;
	int year;
	int hour;
	int minute;
	int num_cpus;
	int num_workers;
	int cpu;
	int worker;
	uint64_t start;
	int nsamples = 0;
	int trash;
	struct event_set* es;
	struct state_event se;
	struct comm_event ce;
	struct single_event sge;

	if(!(fp  = fopen(file, "r")))
		goto out;

	fscanf(fp, "#Paraver (%d/%d/%d at %d:%d):%"PRIu64":1(%d):1:1(%d:1)\n", &day, &month, &year, &hour, &minute, &start, &num_cpus, &num_workers);
	printf("#Paraver (%d/%d/%d at %d:%d):%"PRIu64":1(%d):1:1(%d:1)\n", day, month, year, hour, minute, start, num_cpus, num_workers);


	while(!feof(fp)) {
		int sample_type;
		fscanf(fp, "%d:", &sample_type);

		switch(sample_type) {
			case 1:
				fscanf(fp, "%d:1:1:%d:%"PRIu64":%"PRIu64":%d\n", &cpu, &worker, &se.start, &se.end, &se.state);
				cpu--;
				es = multi_event_set_find_alloc_cpu(mes, cpu);
				event_set_add_state_event(es, &se);
				break;

			case 2:
				fscanf(fp, "%d:1:1:%d:%"PRIu64":%d:1\n", &cpu, &worker, &sge.time, &trash);
				cpu--;
				sge.type = SINGLE_TYPE_TCREATE;
				es = multi_event_set_find_alloc_cpu(mes, cpu);
				event_set_add_single_event(es, &sge);
				break;
			case 3:
				fscanf(fp, "%d:1:1:%d:%"PRIu64":%"PRIu64":%d:1:1:%d:%"PRIu64":%"PRIu64":%d:1\n",
				       &cpu,
				       &worker,
				       &ce.time,
				       &start,
				       &ce.dst_cpu,
				       &ce.dst_worker,
				       &start,
				       &start,
				       &ce.size);
				cpu--;
				ce.dst_cpu--;
				ce.dst_worker--;
				ce.type = COMM_TYPE_UNKNOWN;
				es = multi_event_set_find_alloc_cpu(mes, cpu);
				event_set_add_comm_event(es, &ce);
				break;
		}

		nsamples++;
	}

	res = 0;
	fclose(fp);
out:
	printf("OUT with code %d, file = %s, nsamples = %d\n", res, file, nsamples);

	multi_event_set_sort_by_cpu(mes);

	for(int i = 0; i < mes->num_sets; i++)
		event_set_sort_comm(&mes->sets[i]);

	return res;

}
