/**
 * Copyright (C) 2016 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef OMP_TASK_TREEVIEW_H
#define OMP_TASK_TREEVIEW_H

#include <gtk/gtk.h>
#include "filter.h"
#include "omp_task.h"

G_BEGIN_DECLS

enum omp_task_treeview_columns {
	OMP_TASK_TREEVIEW_COL_FILTER = 0,
	OMP_TASK_TREEVIEW_COL_COLOR,
	OMP_TASK_TREEVIEW_COL_ADDR,
	OMP_TASK_TREEVIEW_COL_START,
	OMP_TASK_TREEVIEW_COL_END,
	OMP_TASK_TREEVIEW_COL_CPU,
	OMP_TASK_TREEVIEW_COL_SYMBOL,
	OMP_TASK_TREEVIEW_COL_SOURCE_FILE,
	OMP_TASK_TREEVIEW_COL_SOURCE_LINE,
	OMP_TASK_TREEVIEW_COL_POINTER,
	OMP_TASK_TREEVIEW_COL_LEVEL,
	OMP_TASK_TREEVIEW_COL_NUM
};

GtkWidget* omp_task_treeview_init(GtkTreeView* omp_task_treeview);
void omp_task_treeview_fill(GtkTreeView* omp_task_treeview, struct omp_task* omp_tasks, int num_omp_tasks);
void omp_task_treeview_build_filter(GtkTreeView* omp_task_treeview, struct filter* filter);

G_END_DECLS

#endif
