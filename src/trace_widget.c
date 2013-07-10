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

#include "trace_widget.h"
#include "marshal.h"
#include "color.h"
#include <math.h>
#include <inttypes.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

gint gtk_trace_signals[GTK_TRACE_MAX_SIGNALS] = { 0 };

gint gtk_trace_scroll_event(GtkWidget* widget, GdkEventScroll* event);
gint gtk_trace_button_press_event(GtkWidget* widget, GdkEventButton* event);
gint gtk_trace_button_release_event(GtkWidget* widget, GdkEventButton* event);
gint gtk_trace_motion_event(GtkWidget* widget, GdkEventMotion* event);

static double state_colors[][3] = {{COL_NORM(117.0), COL_NORM(195.0), COL_NORM(255.0)},
				   {COL_NORM(  0.0), COL_NORM(  0.0), COL_NORM(255.0)},
				   {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(255.0)},
				   {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(  0.0)},
				   {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(174.0)},
				   {COL_NORM(179.0), COL_NORM(  0.0), COL_NORM(  0.0)},
				   {COL_NORM(  0.0), COL_NORM(255.0), COL_NORM(  0.0)},
				   {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(  0.0)},
				   {COL_NORM(235.0), COL_NORM(  0.0), COL_NORM(  0.0)}};

static double comm_colors[][3] = {{COL_NORM(255.0), COL_NORM(255.0), COL_NORM(  0.0)},
				  {COL_NORM(225.0), COL_NORM(137.0), COL_NORM(  0.0)},
				  {COL_NORM( 23.0), COL_NORM( 95.0), COL_NORM(  0.0)},
				  {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(255.0)}};

static double highlight_color[3] = {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(  0.0)};

GtkWidget* gtk_trace_new(struct multi_event_set* mes)
{
	GtkTrace *g = gtk_type_new(gtk_trace_get_type());
	g->left = 0;
	g->right = 100000000;
	g->axis_width = 70;
	g->tick_width = 10;
	g->minor_tick_width = 5;
	g->scroll_amount = 0.1;
	g->cpu_scroll_px = 10;
	g->zoom_factor = 1.1;
	g->event_sets = mes;
	g->cpu_height = 20;
	g->cpu_offset = 0;
	g->mode = GTK_TRACE_MODE_NORMAL;
	g->draw_states = 1;
	g->draw_comm = 0;
	g->draw_comm_size = 0;
	g->draw_steals = 1;
	g->draw_pushes = 1;
	g->draw_data_reads = 1;
	g->draw_counters = 0;

	g->draw_single_events = 0;
	g->back_buffer = NULL;
	g->double_buffering = 0;
	g->filter = NULL;
	g->highlight_state_event = NULL;

	return GTK_WIDGET(g);
}

GtkType gtk_trace_get_type(void)
{
	static GtkType gtk_trace_type = 0;

	if (!gtk_trace_type) {
		static const GtkTypeInfo gtk_trace_type_info = {
			"GtkTrace",
			sizeof(GtkTrace),
			sizeof(GtkTraceClass),
			(GtkClassInitFunc) gtk_trace_class_init,
			(GtkObjectInitFunc) gtk_trace_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		gtk_trace_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_trace_type_info);
	}

	return gtk_trace_type;
}

void gtk_trace_destroy(GtkObject *object)
{
	GtkTrace *trace;
	GtkTraceClass *class;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_TRACE(object));

	trace = GTK_TRACE(object);

	class = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(class)->destroy) {
		(* GTK_OBJECT_CLASS(class)->destroy)(object);
	}
}

void gtk_trace_init(GtkTrace *trace)
{
}

void gtk_trace_class_init(GtkTraceClass *class)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;

	widget_class = (GtkWidgetClass *) class;
	object_class = (GtkObjectClass *) class;

	widget_class->realize = gtk_trace_realize;
	widget_class->size_request = gtk_trace_size_request;
	widget_class->size_allocate = gtk_trace_size_allocate;
	widget_class->expose_event = gtk_trace_expose;
	widget_class->button_press_event = gtk_trace_button_press_event;
	widget_class->button_release_event = gtk_trace_button_release_event;

	object_class->destroy = gtk_trace_destroy;

	gtk_trace_signals[GTK_TRACE_BOUNDS_CHANGED] =
                g_signal_new("bounds-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(GtkTraceClass, bounds_changed),
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__DOUBLE_DOUBLE,
                             G_TYPE_NONE, 2,
                             G_TYPE_DOUBLE, G_TYPE_DOUBLE);

	gtk_trace_signals[GTK_TRACE_STATE_EVENT_UNDER_POINTER_CHANGED] =
                g_signal_new("state-event-under-pointer-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(GtkTraceClass, bounds_changed),
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__POINTER_INT_INT,
                             G_TYPE_NONE, 3,
                             G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT);

	gtk_trace_signals[GTK_TRACE_STATE_EVENT_SELECTION_CHANGED] =
                g_signal_new("state-event-selection-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(GtkTraceClass, bounds_changed),
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__POINTER_INT_INT,
                             G_TYPE_NONE, 3,
                             G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT);

	gtk_trace_signals[GTK_TRACE_YBOUNDS_CHANGED] =
                g_signal_new("ybounds-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(GtkTraceClass, bounds_changed),
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__DOUBLE_DOUBLE,
                             G_TYPE_NONE, 2,
                             G_TYPE_DOUBLE, G_TYPE_DOUBLE);
}

void gtk_trace_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TRACE(widget));
	g_return_if_fail(requisition != NULL);

	requisition->height = 50;
	requisition->height = 50;
}

void gtk_trace_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TRACE(widget));
	g_return_if_fail(allocation != NULL);

	GtkTrace* g = GTK_TRACE(widget);

	widget->allocation = *allocation;

//	cairo_surface_reference(g->back_buffer);

	if (GTK_WIDGET_REALIZED(widget)) {
		if(g->double_buffering) {
			if(g->back_buffer)
				cairo_surface_destroy(g->back_buffer);

			g->back_buffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,  widget->allocation.width, widget->allocation.height);
		}

		gdk_window_move_resize(
			widget->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height
			);
	}
}

void gtk_trace_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TRACE(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
				GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK |  GDK_POINTER_MOTION_HINT_MASK |
				GDK_BUTTON1_MOTION_MASK | GDK_POINTER_MOTION_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	g_signal_connect(G_OBJECT(widget), "scroll-event", G_CALLBACK(gtk_trace_scroll_event), NULL);
	g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(gtk_trace_motion_event), NULL);

	widget->window = gdk_window_new(
		gtk_widget_get_parent_window (widget),
		& attributes, attributes_mask
		);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static inline double gtk_trace_cpu_height(GtkTrace* g)
{
	return g->cpu_height;
}

gboolean gtk_trace_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_TRACE(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	gtk_trace_paint(widget);

	GtkTrace* g = GTK_TRACE(widget);
	double cpu_height = gtk_trace_cpu_height(g);
	double start_cpu = g->cpu_offset;
	double end_cpu = start_cpu + ((g->widget.allocation.height - g->axis_width) / cpu_height);

	g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_YBOUNDS_CHANGED], 0, start_cpu, end_cpu);

	return FALSE;
}

static inline double gtk_trace_cpu_start(GtkTrace* g, int cpu_idx)
{
	double height = gtk_trace_cpu_height(g);
	return cpu_idx*height - height*g->cpu_offset;
}

static inline long double optimal_step_size(long double lower, long double upper, long double* start_with)
{
	long double diff = upper - lower;
	long double log_level = logl(diff) / logl(10);
	long double step_size = powl(10.0, floor(log_level));

	if(diff / step_size <= 3)
		step_size /= 10;

	long double start = step_size * roundl(lower / step_size);

	if(start >= lower)
		*start_with = start;
	else
		*start_with = start + step_size;

	return step_size;
}

static inline long double gtk_trace_pxpv_x(GtkTrace* g)
{
	return ((long double)g->widget.allocation.width - g->axis_width)  / (long double)(g->right - g->left);
}

static inline long double gtk_trace_x_to_screen(GtkTrace* g, long double x)
{
	long double px_per_val = gtk_trace_pxpv_x(g);
	long double norm_val = x - g->left;
	long double norm_px_val = norm_val * px_per_val;
	long double norm_px_w_offs = g->axis_width +  norm_px_val;

	return norm_px_w_offs;
}

static inline long double gtk_trace_screen_x_to_trace(GtkTrace* g, int x)
{
	long double px_per_val = gtk_trace_pxpv_x(g);
	return ((x - g->axis_width) / px_per_val) + g->left;
}

static inline long double gtk_trace_screen_width_to_trace(GtkTrace* g, int w)
{
	long double px_per_val = ((long double)g->widget.allocation.width)  / (g->right - g->left);
	return w / px_per_val;
}

gint gtk_trace_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
	GtkTrace* g = GTK_TRACE(widget);
	long double incr;
	float zoom_factor;

	/* Horizontal shift */
	if((event->state & GDK_SHIFT_MASK) && !(event->state & GDK_CONTROL_MASK)) {
		incr = (g->right - g->left) * g->scroll_amount;

		if(event->direction == GDK_SCROLL_UP)
			incr *= -1;

		g->left += incr;
		g->right += incr;

		g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_BOUNDS_CHANGED], 0, (double)g->left, (double)g->right);
		gtk_widget_queue_draw(widget);
	}

	/* Zoom for cpu_height */
	if(!(event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK)) {
		if(event->direction == GDK_SCROLL_DOWN)
			zoom_factor = g->zoom_factor;
		else
			zoom_factor = 1.0 / g->zoom_factor;

		g->cpu_height /= zoom_factor;

		double cpu_height = gtk_trace_cpu_height(g);
		double start_cpu = g->cpu_offset;
		double end_cpu = start_cpu + ((g->widget.allocation.height - g->axis_width) / cpu_height);
		g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_YBOUNDS_CHANGED], 0, start_cpu, end_cpu);
		gtk_widget_queue_draw(widget);
	}

	/* Vertical shift */
	if((!(event->state & GDK_CONTROL_MASK) && event->x < g->axis_width) ||
	   ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK)))
	{
		double cpu_height = gtk_trace_cpu_height(g);

		if(event->direction == GDK_SCROLL_DOWN)
			g->cpu_offset += g->cpu_scroll_px / cpu_height;
		else
			g->cpu_offset -= g->cpu_scroll_px / cpu_height;

		if(g->cpu_offset < 0)
			g->cpu_offset = 0;

		if(g->cpu_offset > g->event_sets->num_sets-1)
			g->cpu_offset = g->event_sets->num_sets-1;

		double start_cpu = g->cpu_offset;
		double end_cpu = start_cpu + ((g->widget.allocation.height - g->axis_width) / cpu_height);

		g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_YBOUNDS_CHANGED], 0, start_cpu, end_cpu);
		gtk_widget_queue_draw(widget);
	}

	/* Normal zoom */
	if(event->x > g->axis_width && event->y < g->widget.allocation.height - g->axis_width) {
		if(!(event->state & GDK_CONTROL_MASK) && !(event->state & GDK_SHIFT_MASK)) {
			long double curr_x = gtk_trace_screen_x_to_trace(g, event->x);
			long double width = g->right - g->left;

			if(event->direction == GDK_SCROLL_DOWN)
				zoom_factor = g->zoom_factor;
			else
				zoom_factor = 1.0 / g->zoom_factor;

			g->left -= (width / 2) * zoom_factor - width / 2;
			g->right += (width / 2) * zoom_factor - width / 2;

			long double gxstar = gtk_trace_screen_x_to_trace(g, event->x);
			g->left -= gxstar - curr_x;
			g->right -= gxstar - curr_x;

			g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_BOUNDS_CHANGED], 0, (double)g->left, (double)g->right);
			gtk_widget_queue_draw(widget);
		}
	}

	return 1;
}

gint gtk_trace_button_press_event(GtkWidget* widget, GdkEventButton *event)
{
	GtkTrace* g = GTK_TRACE(widget);

	if(event->button != 1)
		return TRUE;

	switch(g->mode) {
		case GTK_TRACE_MODE_NORMAL:
			g->mode = GTK_TRACE_MODE_NAVIGATE;
			g->last_mouse_x = event->x;
			g->last_mouse_y = event->y;
			g->moved_during_navigation = 0;
			break;
		default:
			break;
	}

	return TRUE;
}

gint gtk_trace_button_release_event(GtkWidget *widget, GdkEventButton* event)
{
	struct state_event* se;
	int worker, cpu;
	GtkTrace* g = GTK_TRACE(widget);

	if(event->button != 1)
		return TRUE;

	g->mode = GTK_TRACE_MODE_NORMAL;

	/* Normal click? */
	if(!g->moved_during_navigation) {
		se = gtk_trace_get_state_event_at(widget, event->x, event->y, &cpu, &worker);

		if(se && (!g->filter || filter_has_task(g->filter, se->active_task))) {
			g->highlight_state_event = se;
			g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_STATE_EVENT_SELECTION_CHANGED], 0, se, cpu, worker);
			gtk_widget_queue_draw(widget);
		}
	}

	return TRUE;
}

gint gtk_trace_motion_event(GtkWidget* widget, GdkEventMotion* event)
{
	GtkTrace* g = GTK_TRACE(widget);
	double diff_x = event->x - g->last_mouse_x;
	struct state_event* se;
	int worker, cpu;

	switch(g->mode) {
		case GTK_TRACE_MODE_NAVIGATE:
			g->left -= gtk_trace_screen_width_to_trace(g, diff_x);
			g->right -= gtk_trace_screen_width_to_trace(g, diff_x);

			g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_BOUNDS_CHANGED], 0, (double)g->left, (double)g->right);

			g->last_mouse_x = event->x;
			g->last_mouse_y = event->y;

			g->moved_during_navigation = 1;
			gtk_widget_queue_draw(widget);
			break;
		default:
			se = gtk_trace_get_state_event_at(widget, event->x, event->y, &cpu, &worker);
			if(se && (!g->filter || filter_has_task(g->filter, se->active_task)))
				g_signal_emit(widget, gtk_trace_signals[GTK_TRACE_STATE_EVENT_UNDER_POINTER_CHANGED], 0, se, cpu, worker);

			break;
	}

	return TRUE;
}

void gtk_trace_paint_background(GtkTrace* g, cairo_t* cr)
{
	double cpu_height = gtk_trace_cpu_height(g);
	double cpu_start;

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, g->widget.allocation.width, g->widget.allocation.height);
	cairo_fill(cr);

	cairo_rectangle(cr, 0, 0, g->widget.allocation.width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);
	for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
		if(cpu_idx % 2 == 0) {
			cpu_start = gtk_trace_cpu_start(g, cpu_idx);
			cairo_set_source_rgba(cr, .5, .5, .5, .5);
			cairo_rectangle(cr, 0, cpu_start, g->widget.allocation.width, cpu_height);
			cairo_fill(cr);
		}
	}

	cairo_reset_clip(cr);
}

void gtk_trace_paint_axes(GtkTrace* g, cairo_t* cr)
{
	char buf[20];
	cairo_text_extents_t extents;
	double cpu_start;

	cairo_set_source_rgb(cr, .5, .5, 0.0);
	cairo_set_line_width(cr, 1.0);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 8);

	cairo_rectangle(cr, 0, 0, g->axis_width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);

	/* Y labels */
	double cpu_height = gtk_trace_cpu_height(g);

	for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
		cpu_start = gtk_trace_cpu_start(g, cpu_idx);

		snprintf(buf, sizeof(buf), "CPU %d", g->event_sets->sets[cpu_idx].cpu);
		cairo_text_extents(cr, buf, &extents);
		cairo_set_source_rgb(cr, .5, .5, 0.0);
		cairo_move_to(cr, 5, cpu_start + (cpu_height + extents.height) / 2);
		cairo_show_text(cr, buf);
	}

	cairo_reset_clip(cr);

	/* Axis lines */
	cairo_set_source_rgb(cr, .5, .5, 0.0);
	cairo_move_to(cr, g->axis_width + .5, 0);
	cairo_line_to(cr, g->axis_width + .5, g->widget.allocation.height - g->axis_width + 0.5);
	cairo_line_to(cr, g->widget.allocation.width, g->widget.allocation.height - g->axis_width + 0.5);
	cairo_stroke(cr);

	long double start;
	long double step_size = optimal_step_size(g->left, g->right, &start);
	double screen_x, screen_y_top;
	long double minor_screen_x;

	cairo_rectangle(cr, g->axis_width, 0, g->widget.allocation.width - g->axis_width, g->widget.allocation.height);
	cairo_clip(cr);

	/* X labels and ticks */
	for(long double x = start-step_size; x < g->right+step_size; x += step_size) {
		screen_x = gtk_trace_x_to_screen(g, x)+0.5;
		screen_y_top = g->widget.allocation.height - g->axis_width;

		/* Long tick (vertical line over the whole graph) */
		cairo_set_source_rgba(cr, .5, .5, .5, .5);
		cairo_move_to(cr, screen_x, 0);
		cairo_line_to(cr, screen_x, screen_y_top);
		cairo_stroke(cr);

		/* Tick */
		cairo_set_source_rgb(cr, .5, .5, 0.0);
		cairo_move_to(cr, screen_x, screen_y_top);
		cairo_line_to(cr, screen_x, screen_y_top + g->tick_width);
		cairo_stroke(cr);

		for(long double i = 1; i < 10; i++) {
			/* Minor tick */
			minor_screen_x = gtk_trace_x_to_screen(g, x+i*step_size/10)+0.5;
			cairo_move_to(cr, minor_screen_x, g->widget.allocation.height - g->axis_width);
			cairo_line_to(cr, minor_screen_x, screen_y_top + g->minor_tick_width);
			cairo_stroke(cr);
		}


		/* Label */
		cairo_save(cr);
		cairo_translate(cr, 0, 0);
		cairo_rotate(cr, 3*M_PI/2);
		snprintf(buf, 20, "%.3Le", x);
		cairo_move_to(cr, -g->widget.allocation.height+5, screen_x);
		cairo_show_text(cr, buf);
		cairo_restore(cr);
	}

	cairo_reset_clip(cr);
}

void gtk_trace_paint_states(GtkTrace* g, cairo_t* cr)
{
	double cpu_height = gtk_trace_cpu_height(g);

	cairo_rectangle(cr, g->axis_width, 0, g->widget.allocation.width - g->axis_width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);

	cairo_set_source_rgb(cr, 1.0, 0, 0);
	int num_events_drawn = 0;

	for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
		long double last_start = 0;
		long double last_end;
		int last_major_state = -1;
		double cpu_start = gtk_trace_cpu_start(g, cpu_idx);

		for(int px = g->axis_width; px < g->widget.allocation.width; px++) {
			int major_state;
			long double start = gtk_trace_screen_x_to_trace(g, px);
			long double end = gtk_trace_screen_x_to_trace(g, px+1);

			if(start < 0)
				continue;

			int has_major = event_set_get_major_state(&g->event_sets->sets[cpu_idx], g->filter, start, end, &major_state);

			if(last_major_state != -1) {
				if((has_major && last_major_state != major_state) || !has_major) {
					cairo_set_source_rgb(cr, state_colors[last_major_state][0], state_colors[last_major_state][1], state_colors[last_major_state][2]);
					cairo_rectangle(cr, last_start, cpu_start, px-last_start, cpu_height);
					cairo_fill(cr);
					num_events_drawn++;
				}
			}

			if(has_major && last_major_state != major_state) {
				last_major_state = major_state;
				last_start = px;
			}

			if(!has_major)
				last_major_state = -1;
			else
				last_end = px;
		}

		if(last_major_state != -1) {
			cairo_set_source_rgb(cr, state_colors[last_major_state][0], state_colors[last_major_state][1], state_colors[last_major_state][2]);
			cairo_rectangle(cr, last_start, cpu_start, last_end - last_start, cpu_height);
			cairo_fill(cr);
			num_events_drawn++;
		}

		if(g->highlight_state_event &&
		   g->highlight_state_event >= g->event_sets->sets[cpu_idx].state_events &&
		   g->highlight_state_event <= &g->event_sets->sets[cpu_idx].state_events[g->event_sets->sets[cpu_idx].num_state_events-1] &&
		   (!g->filter || filter_has_task(g->filter, g->highlight_state_event->active_task)))
		{
			if(g->highlight_state_event->start <= g->right && g->highlight_state_event->end >= g->left) {
				double x_start = gtk_trace_x_to_screen(g, g->highlight_state_event->start);
				double x_end = gtk_trace_x_to_screen(g, g->highlight_state_event->end);

				if(x_start < g->axis_width)
					x_start = 0;

				if(x_end > g->widget.allocation.width)
					x_end = g->widget.allocation.width;

				double width = x_end - x_start;

				if(width < 1)
					width = 1;

				cairo_set_source_rgb(cr, highlight_color[0], highlight_color[1], highlight_color[2]);
				cairo_rectangle(cr, x_start, cpu_start, width, cpu_height);
				cairo_fill(cr);
			}
		}
	}

	if(cpu_height > 3) {
		cairo_set_source_rgb(cr, 0, 0, 0);
		for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
			double cpu_start = gtk_trace_cpu_start(g, cpu_idx);

			cairo_move_to(cr, g->axis_width, cpu_start+0.5);
			cairo_line_to(cr, g->widget.allocation.width, cpu_start+0.5);
			cairo_stroke(cr);
		}
	}

	printf("State events drawn: %d\n", num_events_drawn);

	cairo_reset_clip(cr);
}

void gtk_trace_paint_comm(GtkTrace* g, cairo_t* cr)
{
	char buffer[20];
	double cpu_height = gtk_trace_cpu_height(g);
	struct coord { int y1; int y2; } lines_painted[g->widget.allocation.width];
	cairo_text_extents_t extents;

	memset(lines_painted, 0, sizeof(lines_painted[0])*g->widget.allocation.width);

	cairo_rectangle(cr, g->axis_width, 0, g->widget.allocation.width - g->axis_width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 8);

	cairo_set_line_width (cr, 1);

	int num_events_drawn = 0;

	for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
		int comm_event = event_set_get_first_comm_in_interval(&g->event_sets->sets[cpu_idx], (g->left > 0) ? g->left : 0, g->right);
		double cpu_start = gtk_trace_cpu_start(g, cpu_idx);

		if(comm_event != -1) {
			for(; comm_event < g->event_sets->sets[cpu_idx].num_comm_events; comm_event++) {
				uint64_t time = g->event_sets->sets[cpu_idx].comm_events[comm_event].time;

				if(g->event_sets->sets[cpu_idx].comm_events[comm_event].time > g->right)
					break;

				if((long double)time >= g->left && (long double)time <= g->right)
				{
					int dst_cpu = g->event_sets->sets[cpu_idx].comm_events[comm_event].dst_cpu;
					int comm_type = g->event_sets->sets[cpu_idx].comm_events[comm_event].type;
					uint64_t comm_size = g->event_sets->sets[cpu_idx].comm_events[comm_event].size;

					if(comm_type == COMM_TYPE_STEAL && !g->draw_steals)
						continue;
					else if(comm_type == COMM_TYPE_PUSH && !g->draw_pushes)
						continue;
					else if(comm_type == COMM_TYPE_DATA_READ && !g->draw_data_reads)
						continue;

					if(g->filter) {
						if(comm_type == COMM_TYPE_DATA_READ &&
						   !filter_has_task(g->filter, g->event_sets->sets[cpu_idx].comm_events[comm_event].active_task))
						{
							struct event_set* dst_es = multi_event_set_find_cpu(g->event_sets, dst_cpu);
							int idx = event_set_get_enclosing_state(dst_es, time);

							if(idx == -1 || !filter_has_task(g->filter, dst_es->state_events[idx].active_task))
								continue;
						}
					}

					long double screen_x = roundl(gtk_trace_x_to_screen(g, time));
					int dst_cpu_idx = multi_event_set_find_cpu_idx(g->event_sets, dst_cpu);
					double dst_cpu_start = gtk_trace_cpu_start(g, dst_cpu_idx);

					int y1 = (cpu_idx < dst_cpu_idx) ? cpu_idx : dst_cpu_idx;
					int y2 = (cpu_idx < dst_cpu_idx) ? dst_cpu_idx : cpu_idx;

					cairo_set_source_rgb(cr, comm_colors[comm_type][0], comm_colors[comm_type][1], comm_colors[comm_type][2]);
					if(cpu_idx != dst_cpu_idx) {
						if(!(lines_painted[(int)screen_x].y1 <= y1 && lines_painted[(int)screen_x].y2 >= y2)) {
							if(g->draw_comm_size) {
								snprintf(buffer, sizeof(buffer), "%"PRIu64, comm_size);
								cairo_text_extents(cr, buffer, &extents);

								cairo_save(cr);
								cairo_translate(cr, 0, 0);
								cairo_rotate(cr, 3*M_PI/2);
								cairo_move_to(cr, -((cpu_start+dst_cpu_start)/2.0 + extents.width/2.0), screen_x-3);
								cairo_show_text(cr, buffer);
								cairo_restore(cr);
							}

							cairo_move_to(cr, screen_x+0.5, cpu_start + cpu_height/2);
							cairo_line_to(cr, screen_x+0.5, dst_cpu_start + cpu_height/2);
							cairo_stroke(cr);
							num_events_drawn++;

							if(lines_painted[(int)screen_x].y1 < y1)
								lines_painted[(int)screen_x].y1 = y1;

							if(lines_painted[(int)screen_x].y2 > y2)
								lines_painted[(int)screen_x].y2 = y2;
						}
					} else {
						double triangle_height = cpu_height - 2;
						double triangle_width = 8;

						cairo_move_to(cr, screen_x+0.5, cpu_start + cpu_height/2 - triangle_height/2.0);
						cairo_line_to(cr, screen_x+0.5, cpu_start + cpu_height/2 + triangle_height/2.0);
						cairo_line_to(cr, screen_x+0.5+triangle_width, cpu_start + cpu_height/2);
						cairo_move_to(cr, screen_x+0.5, cpu_start + cpu_height/2 - triangle_height/2.0);
						cairo_fill(cr);

						if(g->draw_comm_size) {
							snprintf(buffer, sizeof(buffer), "%"PRIu64, comm_size);
							cairo_text_extents(cr, buffer, &extents);
							cairo_move_to(cr, screen_x+0.5+triangle_width+3, cpu_start + cpu_height/2 + extents.height / 2.0);
							cairo_show_text(cr, buffer);
						}

						num_events_drawn++;
					}
				}
			}
		}
	}

	printf("Comm events drawn: %d\n", num_events_drawn);
	cairo_reset_clip(cr);
}

void gtk_trace_paint_counters(GtkTrace* g, cairo_t* cr)
{
	struct counter_event_set* ces;
	struct counter_description* cd;
	int event_idx;

	double cpu_height = gtk_trace_cpu_height(g);
	long double screen_x;
	long double screen_y;
	long double last_screen_x;
	long double last_screen_y;
	long double rel_val;
	int line_segments_drawn = 0;

	int64_t min;
	int64_t max;

	long double min_slope;
	long double max_slope;

	cairo_rectangle(cr, g->axis_width, 0, g->widget.allocation.width - g->axis_width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);

	cairo_set_line_width(cr, 1.0);

	for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
		double cpu_start = gtk_trace_cpu_start(g, cpu_idx);

		for(int ctr = 0; ctr < g->event_sets->sets[cpu_idx].num_counter_event_sets; ctr++) {
			ces = &g->event_sets->sets[cpu_idx].counter_event_sets[ctr];
			cd = multi_event_set_find_counter_description_by_index(g->event_sets, ces->counter_index);

			if(g->filter && !filter_has_counter(g->filter, cd))
				continue;

			if(g->filter && g->filter->filter_counter_values) {
				min = g->filter->min;
				max = g->filter->max;
			} else {
				min = cd->min;
				max = cd->max;
			}

			if(g->filter && g->filter->filter_counter_slopes) {
				min_slope = g->filter->min_slope;
				max_slope = g->filter->max_slope;
			} else {
				min_slope = cd->min_slope;
				max_slope = cd->max_slope;
			}

			cairo_set_source_rgb(cr, cd->color_r, cd->color_g, cd->color_b);

			event_idx = counter_event_set_get_event_outside_interval(ces, (g->left > 0) ? g->left : 0, g->right);

			if(event_idx != -1 && event_idx < ces->num_events-1) {
				if(ces->events[event_idx].time >= g->left) {
					screen_x = gtk_trace_x_to_screen(g, ces->events[event_idx].time);

					if(!cd->slope_mode)
						rel_val = (long double)ces->events[event_idx].value / (long double)(max - min);
					else
						rel_val = ces->events[event_idx].slope / (max_slope - min_slope);
				} else {
					if(!cd->slope_mode) {
						long double xdiff = (long double)(ces->events[event_idx+1].time - ces->events[event_idx].time);
						long double ydiff = (long double)(ces->events[event_idx+1].value - ces->events[event_idx].value);
						long double xdiff_invisible = (long double)(g->left - ces->events[event_idx+1].time);
						long double slope = ydiff / xdiff;
						rel_val = ((long double)ces->events[event_idx].value + slope*xdiff_invisible) / (long double)(max - min);
					} else {
						rel_val = ces->events[event_idx].slope / (max_slope - min_slope);
					}

					screen_x = gtk_trace_x_to_screen(g, g->left);
				}

				if(rel_val < 0)
					rel_val = 0;
				else if(rel_val > 1.0)
					rel_val = 1.0;

				screen_y = cpu_start+cpu_height - (rel_val*cpu_height);
				last_screen_x = screen_x;
				last_screen_y = screen_y;

				cairo_move_to(cr, screen_x, screen_y);
				event_idx++;

				for(; event_idx < ces->num_events; event_idx++) {
					if(ces->events[event_idx].time <= g->right) {
						screen_x = gtk_trace_x_to_screen(g, ces->events[event_idx].time);
						if(!cd->slope_mode)
							rel_val = (long double)ces->events[event_idx].value / (long double)(max - min);
						else
							rel_val = ces->events[event_idx].slope / (max_slope - min_slope);
					} else {
						if(!cd->slope_mode) {
							long double xdiff = (long double)(ces->events[event_idx].time - ces->events[event_idx-1].time);
							long double ydiff = (long double)(ces->events[event_idx].value - ces->events[event_idx-1].value);
							long double xdiff_visible = (long double)(ces->events[event_idx].time - g->right);
							long double slope = ydiff / xdiff;
							rel_val = ((long double)ces->events[event_idx-1].value + slope*xdiff_visible) / (long double)(max - min);
						} else {
							rel_val = ces->events[event_idx].slope / (max_slope - min_slope);
						}

						screen_x = gtk_trace_x_to_screen(g, g->right);
					}

					if(rel_val < 0)
						rel_val = 0;
					else if(rel_val > 1.0)
						rel_val = 1.0;

					screen_y = cpu_start+cpu_height - (rel_val*cpu_height);

					if(cd->slope_mode)
						cairo_line_to(cr, last_screen_x, screen_y);

					if(cd->slope_mode || round(screen_x) != round(last_screen_x) || round(screen_y) != round(last_screen_y)) {
						cairo_line_to(cr, screen_x, screen_y);
						line_segments_drawn++;
					}

					last_screen_x = screen_x;
					last_screen_y = screen_y;

					if(ces->events[event_idx].time > g->right)
						break;
				}

				cairo_stroke(cr);
			}
		}
	}

	printf("Line segments drawn: %d\n", line_segments_drawn);
	cairo_reset_clip(cr);
}

void gtk_trace_paint_single_events(GtkTrace* g, cairo_t* cr)
{
	double cpu_height = gtk_trace_cpu_height(g);
	const char* event_chars[] = { "C" };

	cairo_rectangle(cr, g->axis_width, 0, g->widget.allocation.width - g->axis_width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);

	cairo_set_source_rgb(cr, 0, 1.0, 0);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 8);

	for(int cpu_idx = 0; cpu_idx < g->event_sets->num_sets; cpu_idx++) {
		int last_ev_px = -1;
		int single_event = event_set_get_first_single_event_in_interval(&g->event_sets->sets[cpu_idx], (g->left > 0) ? g->left : 0, g->right);
		double cpu_start = gtk_trace_cpu_start(g, cpu_idx);

		if(single_event != -1) {
			for(; single_event < g->event_sets->sets[cpu_idx].num_single_events; single_event++) {
				uint64_t time = g->event_sets->sets[cpu_idx].single_events[single_event].time;
				int type = g->event_sets->sets[cpu_idx].single_events[single_event].type;

				if(g->event_sets->sets[cpu_idx].single_events[single_event].time > g->right)
					break;

				if(g->filter && !filter_has_task(g->filter, g->event_sets->sets[cpu_idx].single_events[single_event].active_task))
					continue;

				long double screen_x = roundl(gtk_trace_x_to_screen(g, time));
				if(last_ev_px < (int)(screen_x)) {
					last_ev_px = (int)(screen_x);

					cairo_move_to(cr, screen_x,  cpu_start + 3);
					cairo_line_to(cr, screen_x,  cpu_start+cpu_height - 3);
					cairo_stroke(cr);

					cairo_rectangle(cr, screen_x, cpu_start + 3, 4, 4);
					cairo_fill(cr);

					cairo_move_to(cr, screen_x+5,  cpu_start + cpu_height - 3);
					cairo_show_text(cr, event_chars[type]);
				}
			}
		}
	}

	cairo_reset_clip(cr);
}

void gtk_trace_set_filter(GtkWidget *widget, struct filter* f)
{
	GtkTrace* g = GTK_TRACE(widget);
	g->filter = f;
}

struct filter* gtk_trace_get_filter(GtkWidget *widget)
{
	GtkTrace* g = GTK_TRACE(widget);
	return g->filter;
}

void gtk_trace_set_highlighted_state_event(GtkWidget *widget, struct state_event* se)
{
	GtkTrace* g = GTK_TRACE(widget);
	g->highlight_state_event = se;
}

struct state_event* gtk_trace_get_highlighted_state_event(GtkWidget *widget)
{
	GtkTrace* g = GTK_TRACE(widget);
	return g->highlight_state_event;
}

void gtk_trace_set_bounds(GtkWidget *widget, long double left, long double right)
{
	GtkTrace* g = GTK_TRACE(widget);
	g->left = left;
	g->right = right;
	gtk_widget_queue_draw(widget);
}

void gtk_trace_set_cpu_offset(GtkWidget *widget, long double cpu_offset)
{
	GtkTrace* g = GTK_TRACE(widget);
	g->cpu_offset = cpu_offset;
	gtk_widget_queue_draw(widget);
}

void gtk_trace_get_bounds(GtkWidget *widget, long double* left, long double* right)
{
	GtkTrace* g = GTK_TRACE(widget);
	*left = g->left;
	*right = g->right;
}

void gtk_trace_set_left(GtkWidget *widget, long double left)
{
	GtkTrace* g = GTK_TRACE(widget);
	long double oldw = g->right - g->left;
	gtk_trace_set_bounds(widget, left, left + oldw);
}

void gtk_trace_set_right(GtkWidget *widget, long double right)
{
	GtkTrace* g = GTK_TRACE(widget);
	long double oldw = g->right - g->left;
	gtk_trace_set_bounds(widget, right - oldw, right);
}

void gtk_trace_set_draw_states(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_states);
	g->draw_states = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_draw_comm(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_comm);
	g->draw_comm = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_draw_comm_size(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_comm_size);
	g->draw_comm_size = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_draw_steals(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_steals);
	g->draw_steals = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_draw_pushes(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_pushes);
	g->draw_pushes = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_draw_data_reads(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_data_reads);
	g->draw_data_reads = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}


void gtk_trace_set_draw_single_events(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_single_events);
	g->draw_single_events = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_draw_counters(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_counters);
	g->draw_counters = val;

	if(needs_redraw)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_set_double_buffering(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int value_changed = (val != g->double_buffering);

	g->double_buffering = val;

	if(g->back_buffer) {
		cairo_surface_destroy(g->back_buffer);
		g->back_buffer = NULL;
	}

	if(g->double_buffering && value_changed) {
		g->back_buffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
							    widget->allocation.width,
							    widget->allocation.height);
	}

	if(value_changed)
		gtk_widget_queue_draw(widget);
}

void gtk_trace_paint(GtkWidget *widget)
{
	if(!gtk_widget_is_drawable(widget))
		return;

	cairo_t* cr;

	GtkTrace* g = GTK_TRACE(widget);
	if(g->double_buffering)
		cr = cairo_create(g->back_buffer);
	else
		cr = gdk_cairo_create(widget->window);

	/* Clear background */
	gtk_trace_paint_background(g, cr);

	/* Draw events */
	if(g->draw_states)
		gtk_trace_paint_states(g, cr);

	if(g->draw_counters)
		gtk_trace_paint_counters(g, cr);

	if(g->draw_single_events)
		gtk_trace_paint_single_events(g, cr);

	if(g->draw_comm)
		gtk_trace_paint_comm(g, cr);

	/* Draw axes */
	gtk_trace_paint_axes(g, cr);

	if(g-> double_buffering) {
		cairo_t* crw = gdk_cairo_create(widget->window);
		cairo_set_source_surface(crw, g->back_buffer, 0, 0);
		cairo_paint(crw);
		cairo_destroy(crw);
	}

	cairo_destroy(cr);
}

double gtk_trace_get_time_at(GtkWidget *widget, int x)
{
	GtkTrace* g = GTK_TRACE(widget);
	double pxwidth = widget->allocation.width - g->axis_width;
	double width = g->right - g->left;
	double xrel = x - g->axis_width;

	return g->left + (xrel / pxwidth) * width;
}

struct state_event* gtk_trace_get_state_event_at(GtkWidget *widget, int x, int y, int* cpu, int* worker)
{
	GtkTrace* g = GTK_TRACE(widget);
	int worker_pointer;
	int idx;
	double cpu_height = gtk_trace_cpu_height(g);
	double time;

	if(x < g->axis_width || y > widget->allocation.height - g->axis_width)
		return NULL;

	worker_pointer = (y+cpu_height*g->cpu_offset) / cpu_height;

	if(worker_pointer >= g->event_sets->num_sets)
		return NULL;

	time = gtk_trace_get_time_at(widget, x);
	idx = event_set_get_enclosing_state(&g->event_sets->sets[worker_pointer], time);

	if(idx != -1) {
		if(worker)
			*worker = worker_pointer;
		if(cpu)
			*cpu = g->event_sets->sets[worker_pointer].cpu;

		return &g->event_sets->sets[worker_pointer].state_events[idx];
	}

	return NULL;
}
