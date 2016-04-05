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

#include "filter.h"
#include "task.h"
#include "frame.h"
#include <stdlib.h>

void filter_sort_tasks(struct filter* f)
{
	qsort(f->tasks, f->num_tasks,
	      sizeof(struct task*), compare_tasksp);
}

int filter_has_task(struct filter* f, struct task* t)
{
	if(!f || !f->filter_tasks)
		return 1;

	return (bsearch(&t, f->tasks,
			f->num_tasks, sizeof(struct task*),
			compare_tasksp)
		!= NULL);
}

void filter_sort_ofcps(struct filter* f)
{
	qsort(f->ofcps, f->num_ofcps,
	      sizeof(struct omp_for_chunk_set_part*), compare_ofcpsp);
}

int filter_has_ofcp(struct filter* f, struct omp_for_chunk_set_part* ofcp)
{
	if(!f || !f->filter_ofcps)
		return 1;

	return (bsearch(&ofcp, f->ofcps,
			f->num_ofcps, sizeof(struct omp_for_chunk_set_part*),
			compare_ofcpsp)
		!= NULL);
}

void filter_sort_otps(struct filter* f)
{
	qsort(f->otps, f->num_otps,
	      sizeof(struct omp_task_part*), compare_otpsp);
}

int filter_has_otp(struct filter* f, struct omp_task_part* otp)
{
	if(!f || !f->filter_otps)
		return 1;

	return (bsearch(&otp, f->otps,
			f->num_otps, sizeof(struct omp_task_part*),
			compare_otpsp)
		!= NULL);
}

void filter_sort_frames(struct filter* f)
{
	qsort(f->frames, f->num_frames,
	      sizeof(struct frame*), compare_framesp);
}

int filter_has_frame(struct filter* f, struct frame* fr)
{
	if(!f || !f->filter_frames)
		return 1;

	if(!filter_has_frame_numa_node(f, fr->numa_node))
		return 0;

	return (bsearch(&fr, f->frames,
			f->num_frames, sizeof(struct frame*),
			compare_framesp)
		!= NULL);
}
