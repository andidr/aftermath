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

#ifndef MATRIX_WIDGET_H
#define MATRIX_WIDGET_H

#include <gtk/gtk.h>
#include "intensity_matrix.h"

G_BEGIN_DECLS

#define GTK_MATRIX(obj) GTK_CHECK_CAST(obj, gtk_matrix_get_type (), GtkMatrix)
#define GTK_MATRIX_CLASS(class) GTK_CHECK_CLASS_CAST(class, gtk_matrix_get_type(), GtkCpuClass)
#define GTK_IS_MATRIX(obj) GTK_CHECK_TYPE(obj, gtk_matrix_get_type())

typedef struct _GtkMatrix GtkMatrix;
typedef struct _GtkMatrixClass GtkMatrixClass;

enum gtk_matrix_signals {
	GTK_MATRIX_MAX_SIGNALS
};

struct _GtkMatrix {
	GtkWidget widget;
	double min_threshold;
	double max_threshold;
	struct intensity_matrix* matrix;
};

struct _GtkMatrixClass {
	GtkWidgetClass parent_class;
};

void gtk_matrix_destroy(GtkObject *object);
GtkWidget* gtk_matrix_new(void);
GtkType gtk_matrix_get_type(void);
void gtk_matrix_class_init(GtkMatrixClass *class);
void gtk_matrix_size_request(GtkWidget *widget, GtkRequisition *requisition);
void gtk_matrix_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
void gtk_matrix_realize(GtkWidget *widget);
gboolean gtk_matrix_expose(GtkWidget *widget, GdkEventExpose *event);
void gtk_matrix_init(GtkMatrix *matrix);
void gtk_matrix_paint(GtkWidget *widget);
void gtk_matrix_set_data(GtkWidget *widget, struct intensity_matrix* m);
void gtk_matrix_set_min_threshold(GtkWidget *widget, double val);
void gtk_matrix_set_max_threshold(GtkWidget *widget, double val);

extern gint gtk_matrix_signals[GTK_MATRIX_MAX_SIGNALS];

G_END_DECLS

#endif
