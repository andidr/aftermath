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

G_BEGIN_DECLS

#define GTK_IS_OMP_TREEVIEW(obj) GTK_CHECK_TYPE(obj, gtk_omp_treeview_get_type())

typedef struct _GtkOmpTreeViewClass GtkOmpTreeViewClass;
typedef struct _GtkOmpTreeViewType GtkOmpTreeViewType;

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

enum gtk_omp_for_treeview_signals {
	GTK_OMP_FOR_TREEVIEW_UPDATE_HIGHLIGHTED_PART = 0,
	GTK_OMP_FOR_TREEVIEW_MAX_SIGNALS
};

struct _GtkOmpTreeViewType {
	GtkWidget dummy;
};

struct _GtkOmpTreeViewClass {
	GtkWidgetClass parent_class;

	void (* bounds_changed) (void);
};

GtkWidget* omp_for_treeview_init(GtkTreeView* omp_for_treeview);
void omp_for_treeview_fill(GtkTreeView* omp_for_treeview, struct omp_for* omp_fors, int num_omp_fors);
void omp_for_treeview_build_filter(GtkTreeView* omp_for_treeview, struct filter* filter);
GtkType gtk_omp_treeview_get_type(void);
void gtk_omp_treeview_class_init(GtkOmpTreeViewClass *class);
void gtk_omp_treeview_type_init(GtkOmpTreeViewType *treeviewtype);

#endif
