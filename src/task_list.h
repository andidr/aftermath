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

#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"

enum task_list_columns {
	TASK_LIST_COL_FILTER = 0,
	TASK_LIST_COL_ADDR,
	TASK_LIST_COL_SYMBOL,
	TASK_LIST_COL_SOURCE_FILE,
	TASK_LIST_COL_SOURCE_LINE,
	TASK_LIST_COL_TASK_POINTER,
	TASK_LIST_COL_NUM
};

void task_list_init(GtkTreeView* task_treeview);
void task_list_fill(GtkTreeView* task_treeview, struct task* tasks, int num_tasks);
void task_list_build_filter(GtkTreeView* task_treeview, struct filter* filter);

void task_list_check_all(GtkTreeView* task_treeview);
void task_list_uncheck_all(GtkTreeView* task_treeview);
#endif
