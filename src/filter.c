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

int filter_has_task(struct filter* f, uint64_t work_fn)
{
	struct task key = { .work_fn = work_fn };
	struct task* pkey = &key;

	if(!f->filter_tasks)
		return 1;

	return (bsearch(&pkey, f->tasks,
			f->num_tasks, sizeof(struct task*),
			compare_tasksp)
		!= NULL);
}

void filter_sort_frames(struct filter* f)
{
	qsort(f->frames, f->num_frames,
	      sizeof(struct frame*), compare_framesp);
}

int filter_has_frame(struct filter* f, uint64_t addr)
{
	struct frame key = { .addr = addr };
	struct frame* pkey = &key;

	if(!f->filter_frames)
		return 1;

	return (bsearch(&pkey, f->frames,
			f->num_frames, sizeof(struct frame*),
			compare_framesp)
		!= NULL);
}
