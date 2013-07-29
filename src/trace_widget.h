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
#include "multi_event_set.h"
#include "filter.h"

G_BEGIN_DECLS

#define GTK_TRACE(obj) GTK_CHECK_CAST(obj, gtk_trace_get_type (), GtkTrace)
#define GTK_TRACE_CLASS(class) GTK_CHECK_CLASS_CAST(class, gtk_trace_get_type(), GtkCpuClass)
#define GTK_IS_TRACE(obj) GTK_CHECK_TYPE(obj, gtk_trace_get_type())

typedef struct _GtkTrace GtkTrace;
typedef struct _GtkTraceClass GtkTraceClass;

enum gtk_trace_signals {
	GTK_TRACE_BOUNDS_CHANGED = 0,
	GTK_TRACE_STATE_EVENT_UNDER_POINTER_CHANGED,
	GTK_TRACE_STATE_EVENT_SELECTION_CHANGED,
	GTK_TRACE_YBOUNDS_CHANGED,
	GTK_TRACE_RANGE_SELECTION_CHANGED,
	GTK_TRACE_MAX_SIGNALS
};

enum gtk_trace_modes {
	GTK_TRACE_MODE_NORMAL = 0,
	GTK_TRACE_MODE_NAVIGATE,
	GTK_TRACE_MODE_SELECT_RANGE_START,
	GTK_TRACE_MODE_SELECT_RANGE,
	GTK_TRACE_MODE_ZOOM
};

struct _GtkTrace {
	GtkWidget widget;
	long double left;
	long double right;
	float cpu_height;
	float cpu_offset;
	int axis_width;
	int tick_width;
	int minor_tick_width;
	float scroll_amount;
	float cpu_scroll_px;
	float zoom_factor;
	enum gtk_trace_modes mode;
	int draw_states;
	int draw_comm;
	int draw_comm_size;
	int draw_steals;
	int draw_pushes;
	int draw_data_reads;
	int draw_single_events;
	int draw_counters;
	int moved_during_navigation;

	int64_t range_selection_start;
	int64_t range_selection_end;
	int range_selection;

	int double_buffering;
	cairo_surface_t* back_buffer;

	struct multi_event_set* event_sets;

	double last_mouse_x;
	double last_mouse_y;

	struct filter* filter;
	struct state_event* highlight_state_event;
};

struct _GtkTraceClass {
	GtkWidgetClass parent_class;

	void (* bounds_changed) (GtkTrace *t);
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
void gtk_trace_enter_range_selection_mode(GtkWidget *widget);
void gtk_trace_clear_range_selection(GtkWidget *widget);
void gtk_trace_set_bounds(GtkWidget *widget, long double left, long double right);
void gtk_trace_set_cpu_offset(GtkWidget *widget, long double cpu_offset);
void gtk_trace_set_left(GtkWidget *widget, long double left);
void gtk_trace_set_right(GtkWidget *widget, long double right);
void gtk_trace_get_bounds(GtkWidget *widget, long double* left, long double* right);
void gtk_trace_set_draw_states(GtkWidget *widget, int val);
void gtk_trace_set_draw_comm(GtkWidget *widget, int val);
void gtk_trace_set_draw_comm_size(GtkWidget *widget, int val);
void gtk_trace_set_draw_steals(GtkWidget *widget, int val);
void gtk_trace_set_draw_pushes(GtkWidget *widget, int val);
void gtk_trace_set_draw_data_reads(GtkWidget *widget, int val);
void gtk_trace_set_draw_single_events(GtkWidget *widget, int val);
void gtk_trace_set_draw_counters(GtkWidget *widget, int val);
void gtk_trace_set_double_buffering(GtkWidget *widget, int val);
void gtk_trace_set_filter(GtkWidget *widget, struct filter* f);
struct filter* gtk_trace_get_filter(GtkWidget *widget);
void gtk_trace_set_highlighted_state_event(GtkWidget *widget, struct state_event* se);
struct state_event* gtk_trace_get_highlighted_state_event(GtkWidget *widget);
double gtk_trace_get_time_at(GtkWidget *widget, int x);
struct state_event* gtk_trace_get_state_event_at(GtkWidget *widget, int x, int y, int* cpu, int* worker);

extern gint gtk_trace_signals[GTK_TRACE_MAX_SIGNALS];

G_END_DECLS

#endif
