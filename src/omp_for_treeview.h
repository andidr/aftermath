/**
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
 */

#ifndef OMP_FOR_TREEVIEW_H
#define OMP_FOR_TREEVIEW_H

#include <gtk/gtk.h>
#include "filter.h"
#include "omp_for.h"

enum omp_for_treeview_columns {
	OMP_FOR_TREEVIEW_COL_FILTER = 0,
	OMP_FOR_TREEVIEW_COL_COLOR,
	OMP_FOR_TREEVIEW_COL_ADDR,
	OMP_FOR_TREEVIEW_COL_START,
	OMP_FOR_TREEVIEW_COL_END,
	OMP_FOR_TREEVIEW_COL_CPU,
	OMP_FOR_TREEVIEW_COL_SYMBOL,
	OMP_FOR_TREEVIEW_COL_SOURCE_FILE,
	OMP_FOR_TREEVIEW_COL_SOURCE_LINE,
	OMP_FOR_TREEVIEW_COL_POINTER,
	OMP_FOR_TREEVIEW_COL_LEVEL,
	OMP_FOR_TREEVIEW_COL_NUM
};

void omp_for_treeview_init(GtkTreeView* omp_for_treeview);
void omp_for_treeview_fill(GtkTreeView* omp_for_treeview, struct omp_for* omp_fors, int num_omp_fors);
void omp_for_treeview_build_filter(GtkTreeView* omp_for_treeview, struct filter* filter);

#endif
