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
 */

#include "task_instance.h"

void task_instance_init(struct task_instance* inst, uint64_t start, uint64_t end, struct task* task, int cpu)
{
	inst->start = start;
	inst->end = end;
	inst->task = task;
	inst->num_read_deps = 0;
	inst->remaining_read_deps = 0;
	inst->depth = 0;
	inst->cpu = cpu;

	INIT_LIST_HEAD(&inst->list_nodeps);
	INIT_LIST_HEAD(&inst->list_out_deps);
	INIT_LIST_HEAD(&inst->list_in_deps);
	INIT_LIST_HEAD(&inst->list_selection);
}
