/**
 * Copyright (C) 2014 Quentin Bune <quentin.bunel@gmail.com>
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

#ifndef MULTI_HISTOGRAM_WIDGET_H
#define MULTI_HISTOGRAM_WIDGET_H

#include <gtk/gtk.h>
#include "statistics.h"
#include "export.h"

G_BEGIN_DECLS

#define GTK_MULTI_HISTOGRAM(obj) GTK_CHECK_CAST(obj, gtk_multi_histogram_get_type (), GtkMultiHistogram)
#define GTK_MULTI_HISTOGRAM_CLASS(class) GTK_CHECK_CLASS_CAST(class, gtk_multi_histogram_get_type(), GtkCpuClass)
#define GTK_IS_MULTI_HISTOGRAM(obj) GTK_CHECK_TYPE(obj, gtk_multi_histogram_get_type())

typedef struct _GtkMultiHistogram GtkMultiHistogram;
typedef struct _GtkMultiHistogramClass GtkMultiHistogramClass;

enum gtk_multi_histogram_signals {
	GTK_MULTI_HISTOGRAM_MAX_SIGNALS
};

struct _GtkMultiHistogram {
	GtkWidget widget;
	struct multi_histogram* histograms;
	struct multi_event_set* mes;
};

struct _GtkMultiHistogramClass {
	GtkWidgetClass parent_class;
};

void gtk_multi_histogram_destroy(GtkObject *object);
GtkWidget* gtk_multi_histogram_new(struct multi_event_set* mes);
GtkType gtk_multi_histogram_get_type(void);
void gtk_multi_histogram_class_init(GtkMultiHistogramClass *class);
void gtk_multi_histogram_size_request(GtkWidget *widget, GtkRequisition *requisition);
void gtk_multi_histogram_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
void gtk_multi_histogram_realize(GtkWidget *widget);
gboolean gtk_multi_histogram_expose(GtkWidget *widget, GdkEventExpose *event);
void gtk_multi_histogram_init(GtkMultiHistogram *histogram);
void gtk_multi_histogram_paint(GtkWidget *widget);
void gtk_multi_histogram_set_data(GtkWidget *widget, struct multi_histogram* d);
int gtk_multi_histogram_save_to_file(GtkWidget *widget, enum export_file_format format, const char* filename);

extern gint gtk_multi_histogram_signals[GTK_MULTI_HISTOGRAM_MAX_SIGNALS];

G_END_DECLS

#endif
