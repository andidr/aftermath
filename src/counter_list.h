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

#ifndef COUNTER_LIST_H
#define COUNTER_LIST_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"
#include "counter_description.h"

enum counter_list_columns {
	COUNTER_LIST_COL_FILTER = 0,
	COUNTER_LIST_COL_COLOR,
	COUNTER_LIST_COL_NAME,
	COUNTER_LIST_COL_MODE,
	COUNTER_LIST_COL_MIN,
	COUNTER_LIST_COL_MAX,
	COUNTER_LIST_COL_MIN_SLOPE,
	COUNTER_LIST_COL_MAX_SLOPE,
	COUNTER_LIST_COL_COUNTER_POINTER,
	COUNTER_LIST_COL_NUM
};

void counter_list_init(GtkTreeView* counter_treeview);
void counter_list_append(GtkTreeView* counter_treeview, struct counter_description* counter, gboolean active);
void counter_list_fill(GtkTreeView* counter_treeview, struct counter_description* counters, int num_counters);
void counter_list_clear(GtkTreeView* counter_treeview);
void counter_list_build_filter(GtkTreeView* counter_treeview, struct filter* filter);
void counter_list_update_colors(GtkTreeView* task_treeview);
void counter_list_check_all(GtkTreeView* counter_treeview);
void counter_list_uncheck_all(GtkTreeView* counter_treeview);
struct counter_description* counter_list_get_highlighted_entry(GtkTreeView* counter_treeview);

#endif
