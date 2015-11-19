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

#ifndef HISTOGRAM_WIDGET_H
#define HISTOGRAM_WIDGET_H

#include <gtk/gtk.h>
#include "statistics.h"
#include "export.h"

G_BEGIN_DECLS

#define GTK_HISTOGRAM(obj) GTK_CHECK_CAST(obj, gtk_histogram_get_type (), GtkHistogram)
#define GTK_HISTOGRAM_CLASS(class) GTK_CHECK_CLASS_CAST(class, gtk_histogram_get_type(), GtkCpuClass)
#define GTK_IS_HISTOGRAM(obj) GTK_CHECK_TYPE(obj, gtk_histogram_get_type())

typedef struct _GtkHistogram GtkHistogram;
typedef struct _GtkHistogramClass GtkHistogramClass;

enum gtk_histogram_signals {
	GTK_HISTOGRAM_RANGE_SELECTION_CHANGED = 0,
	GTK_HISTOGRAM_MAX_SIGNALS
};

struct _GtkHistogram {
	GtkWidget widget;
	int moved_during_navigation;

	int64_t range_selection_start;
	int64_t range_selection_end;
	int selecting;
	int selection_enabled;

	struct histogram* histogram;
};

struct _GtkHistogramClass {
	GtkWidgetClass parent_class;
};

void gtk_histogram_destroy(GtkObject *object);
GtkWidget* gtk_histogram_new(void);
GtkType gtk_histogram_get_type(void);
void gtk_histogram_class_init(GtkHistogramClass *class);
void gtk_histogram_size_request(GtkWidget *widget, GtkRequisition *requisition);
void gtk_histogram_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
void gtk_histogram_realize(GtkWidget *widget);
gboolean gtk_histogram_expose(GtkWidget *widget, GdkEventExpose *event);
void gtk_histogram_init(GtkHistogram *histogram);
void gtk_histogram_paint(GtkWidget *widget);
void gtk_histogram_set_data(GtkWidget *widget, struct histogram* d);
int gtk_histogram_save_to_file(GtkWidget *widget, enum export_file_format format, const char* filename);
gint gtk_histogram_button_press_event(GtkWidget* widget, GdkEventButton *event);
gint gtk_histogram_button_release_event(GtkWidget *widget, GdkEventButton* event);
gint gtk_histogram_motion_event(GtkWidget* widget, GdkEventMotion* event);
void gtk_histogram_enable_range_selection(GtkWidget *widget);
void gtk_histogram_disable_range_selection(GtkWidget *widget);

extern gint gtk_histogram_signals[GTK_HISTOGRAM_MAX_SIGNALS];

G_END_DECLS

#endif
