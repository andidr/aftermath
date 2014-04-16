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

#ifndef TASK_INSTANCE_H
#define TASK_INSTANCE_H

#include "./contrib/linux-kernel/list.h"
#include "task.h"
#include "events.h"

struct task_instance {
	struct task* task;
	uint64_t start;
	uint64_t end;
	uint64_t num_read_deps;
	uint64_t remaining_read_deps;

	int depth;
	int reached;

	struct list_head list_nodeps;
	struct list_head list_out_deps;
	struct list_head list_in_deps;
	struct list_head list_all_instances;
};

void task_instance_init(struct task_instance* inst, uint64_t start, uint64_t end, struct task* task);

#endif
