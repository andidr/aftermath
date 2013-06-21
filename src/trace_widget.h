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

#ifndef TRACE_WIDGET_H
#define TRACE_WIDGET_H

#include <gtk/gtk.h>
#include "events.h"

G_BEGIN_DECLS

#define GTK_TRACE(obj) GTK_CHECK_CAST(obj, gtk_trace_get_type (), GtkTrace)
#define GTK_TRACE_CLASS(class) GTK_CHECK_CLASS_CAST(class, gtk_trace_get_type(), GtkCpuClass)
#define GTK_IS_TRACE(obj) GTK_CHECK_TYPE(obj, gtk_trace_get_type())

typedef struct _GtkTrace GtkTrace;
typedef struct _GtkTraceClass GtkTraceClass;

enum gtk_trace_signals {
	GTK_TRACE_MAX_SIGNALS = 0
};

enum gtk_trace_modes {
	GTK_TRACE_MODE_NORMAL = 0,
	GTK_TRACE_MODE_NAVIGATE,
	GTK_TRACE_MODE_ZOOM
};

struct _GtkTrace {
	GtkWidget widget;
	long double left;
	long double right;
	float max_cpu_height;
	int axis_width;
	int tick_width;
	int minor_tick_width;
	float scroll_amount;
	float zoom_factor;
	enum gtk_trace_modes mode;
	int draw_states;
	int draw_comm;
	int draw_single_events;

	int double_buffering;
	cairo_surface_t* back_buffer;

	struct multi_event_set* event_sets;

	double last_mouse_x;
	double last_mouse_y;
};

struct _GtkTraceClass {
	GtkWidgetClass parent_class;
};

void gtk_trace_destroy(GtkObject *object);
GtkWidget* gtk_trace_new(struct multi_event_set* mes);
GtkType gtk_trace_get_type(void);
void gtk_trace_class_init(GtkTraceClass *class);
void gtk_trace_size_request(GtkWidget *widget, GtkRequisition *requisition);
void gtk_trace_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
void gtk_trace_realize(GtkWidget *widget);
gboolean gtk_trace_expose(GtkWidget *widget, GdkEventExpose *event);
void gtk_trace_init(GtkTrace *trace);
void gtk_trace_paint(GtkWidget *widget);
void gtk_trace_set_bounds(GtkWidget *widget, long double left, long double right);
void gtk_trace_set_left(GtkWidget *widget, long double left);
void gtk_trace_set_right(GtkWidget *widget, long double right);
void gtk_trace_get_bounds(GtkWidget *widget, long double* left, long double* right);
void gtk_trace_set_draw_states(GtkWidget *widget, int val);
void gtk_trace_set_draw_comm(GtkWidget *widget, int val);
void gtk_trace_set_draw_single_events(GtkWidget *widget, int val);
void gtk_trace_set_double_buffering(GtkWidget *widget, int val);

extern gint gtk_trace_signals[GTK_TRACE_MAX_SIGNALS];

G_END_DECLS

#endif
