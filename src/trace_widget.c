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
#include <math.h>
#include <inttypes.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

gint gtk_trace_signals[GTK_TRACE_MAX_SIGNALS] = { };

gint gtk_trace_scroll_event(GtkWidget* widget, GdkEventScroll* event);
gint gtk_trace_button_press_event(GtkWidget* widget, GdkEventButton* event);
gint gtk_trace_button_release_event(GtkWidget* widget, GdkEventButton* event);
gint gtk_trace_motion_event(GtkWidget* widget, GdkEventMotion* event);

#define COL_NORM(x) ((x) / 255.0)
static double state_colors[][3] = {{COL_NORM(117.0), COL_NORM(195.0), COL_NORM(255.0)},
				   {COL_NORM(  0.0), COL_NORM(  0.0), COL_NORM(255.0)},
				   {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(255.0)},
				   {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(  0.0)},
				   {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(174.0)},
				   {COL_NORM(179.0), COL_NORM(  0.0), COL_NORM(  0.0)},
				   {COL_NORM(  0.0), COL_NORM(255.0), COL_NORM(  0.0)},
				   {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(  0.0)},
				   {COL_NORM(235.0), COL_NORM(  0.0), COL_NORM(  0.0)}};

GtkWidget* gtk_trace_new(struct multi_event_set* mes)
{
	GtkTrace *g = gtk_type_new(gtk_trace_get_type());
	g->left = 0;
	g->right = 100000000;
	g->axis_width = 70;
	g->tick_width = 10;
	g->minor_tick_width = 5;
	g->scroll_amount = 0.1;
	g->zoom_factor = 1.1;
	g->event_sets = mes;
	g->max_cpu_height = 50;
	g->mode = GTK_TRACE_MODE_NORMAL;
	g->draw_states = 1;
	g->draw_comm = 1;
	g->draw_single_events = 1;
	g->back_buffer = NULL;
	g->double_buffering = 0;

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
}

void gtk_trace_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_TRACE(widget));
	g_return_if_fail(requisition != NULL);

	requisition->height = 200;
	requisition->height = 200;
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
				GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK |  GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON1_MOTION_MASK;

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

gboolean gtk_trace_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_TRACE(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	gtk_trace_paint(widget);

	return FALSE;
}

static inline double gtk_trace_cpu_height(GtkTrace* g)
{
	double height = ((double)g->widget.allocation.height - g->axis_width) / (double)g->event_sets->num_sets;

	if(height > g->max_cpu_height)
		height = g->max_cpu_height;

	return height;
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
	int zoom_x;
	float zoom_factor;

	if(event->state & GDK_SHIFT_MASK) {
		incr = (g->right - g->left) * g->scroll_amount;

		if(event->direction == GDK_SCROLL_UP)
			incr *= -1;

		g->left += incr;
		g->right += incr;
	}

	if(!(event->state & GDK_CONTROL_MASK) && !(event->state & GDK_SHIFT_MASK)) {
		zoom_x = (event->x > g->axis_width);
		long double curr_x = gtk_trace_screen_x_to_trace(g, event->x);
		long double width = g->right - g->left;


		if(event->direction == GDK_SCROLL_DOWN)
			zoom_factor = g->zoom_factor;
		else
			zoom_factor = 1.0 / g->zoom_factor;

		if(zoom_x) {
			g->left -= (width / 2) * zoom_factor - width / 2;
			g->right += (width / 2) * zoom_factor - width / 2;

			long double gxstar = gtk_trace_screen_x_to_trace(g, event->x);
			g->left -= gxstar - curr_x;
			g->right -= gxstar - curr_x;
		}
	}

	gtk_trace_paint(widget);

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
			break;
		default:
			break;
	}

	return TRUE;
}

gint gtk_trace_button_release_event(GtkWidget *widget, GdkEventButton* event)
{
	GtkTrace* g = GTK_TRACE(widget);

	if(event->button != 1)
		return TRUE;

	g->mode = GTK_TRACE_MODE_NORMAL;
	return TRUE;
}

gint gtk_trace_motion_event(GtkWidget* widget, GdkEventMotion* event)
{
	GtkTrace* g = GTK_TRACE(widget);
	double diff_x = event->x - g->last_mouse_x;

	switch(g->mode) {
		case GTK_TRACE_MODE_NAVIGATE:
			g->left -= gtk_trace_screen_width_to_trace(g, diff_x);
			g->right -= gtk_trace_screen_width_to_trace(g, diff_x);

			g->last_mouse_x = event->x;
			g->last_mouse_y = event->y;
			gtk_trace_paint(widget);
			break;
		default:
			break;
	}

	return TRUE;
}

void gtk_trace_paint_background(GtkTrace* g, cairo_t* cr)
{
	double cpu_height = gtk_trace_cpu_height(g);

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, g->widget.allocation.width, g->widget.allocation.height);
	cairo_fill(cr);

	for(int cpu = 0; cpu < g->event_sets->num_sets; cpu++) {
		if(cpu % 2 == 0) {
			cairo_set_source_rgba(cr, .5, .5, .5, .5);
			cairo_rectangle(cr, 0, cpu*cpu_height, g->widget.allocation.width, cpu_height);
			cairo_fill(cr);
		}
	}
}

void gtk_trace_paint_axes(GtkTrace* g, cairo_t* cr)
{
	char buf[20];
	cairo_text_extents_t extents;
	cairo_set_source_rgb(cr, .5, .5, 0.0);
	cairo_set_line_width(cr, 1.0);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 8);

	/* Y labels */
	double cpu_height = gtk_trace_cpu_height(g);

	for(int cpu = 0; cpu < g->event_sets->num_sets; cpu++) {
		snprintf(buf, sizeof(buf), "CPU %d", g->event_sets->sets[cpu].cpu);
		cairo_text_extents(cr, buf, &extents);
		cairo_set_source_rgb(cr, .5, .5, 0.0);
		cairo_move_to(cr, 5, cpu*cpu_height + (cpu_height + extents.height) / 2);
		cairo_show_text(cr, buf);
	}

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
	for(int cpu = 0; cpu < g->event_sets->num_sets; cpu++) {
		int state_event = event_set_get_first_state_in_interval(&g->event_sets->sets[cpu], (g->left > 0) ? g->left : 0, g->right);

		if(state_event != -1) {
			for(; state_event < g->event_sets->sets[cpu].num_state_events; state_event++) {
				if(g->event_sets->sets[cpu].state_events[state_event].start > g->right)
					break;

				if(g->event_sets->sets[cpu].state_events[state_event].start < g->right &&
				   g->event_sets->sets[cpu].state_events[state_event].end > g->left)
				{
					int state = g->event_sets->sets[cpu].state_events[state_event].state;
					uint64_t start = g->event_sets->sets[cpu].state_events[state_event].start;
					uint64_t end = g->event_sets->sets[cpu].state_events[state_event].end;
					long double start_x = gtk_trace_x_to_screen(g, start);

					if(start_x < 0)
						start_x = 0;

					long double end_x   = gtk_trace_x_to_screen(g, end);
					long double width = end_x - start_x;

					if(end_x > g->widget.allocation.width)
						width = g->widget.allocation.width - start_x;

					if(width < 0.1)
						continue;

					cairo_set_source_rgb(cr, state_colors[state][0], state_colors[state][1], state_colors[state][2]);
					cairo_rectangle(cr, (double)start_x+0.5, cpu*cpu_height, width, cpu_height);
					cairo_fill(cr);
					num_events_drawn++;
				}
			}
		}
	}

	cairo_set_source_rgb(cr, 0, 0, 0);
	for(int cpu = 0; cpu < g->event_sets->num_sets; cpu++) {
		cairo_move_to(cr, g->axis_width, cpu*cpu_height+0.5);
		cairo_line_to(cr, g->widget.allocation.width, cpu*cpu_height+0.5);
		cairo_stroke(cr);
	}

	printf("State events drawn: %d\n", num_events_drawn);

	cairo_reset_clip(cr);
}

void gtk_trace_paint_comm(GtkTrace* g, cairo_t* cr)
{
	double cpu_height = gtk_trace_cpu_height(g);
	struct coord { int y1; int y2; } lines_painted[g->widget.allocation.width];

	memset(lines_painted, 0, sizeof(lines_painted[0])*g->widget.allocation.width);

	cairo_rectangle(cr, g->axis_width, 0, g->widget.allocation.width - g->axis_width, g->widget.allocation.height - g->axis_width);
	cairo_clip(cr);

	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb(cr, 1.0, 1.0, 0);
	int num_events_drawn = 0;
	for(int cpu = 0; cpu < g->event_sets->num_sets; cpu++) {
		int comm_event = event_set_get_first_comm_in_interval(&g->event_sets->sets[cpu], (g->left > 0) ? g->left : 0, g->right);

		if(comm_event != -1) {
			for(; comm_event < g->event_sets->sets[cpu].num_comm_events; comm_event++) {
				uint64_t time = g->event_sets->sets[cpu].comm_events[comm_event].time;

				if(g->event_sets->sets[cpu].comm_events[comm_event].time > g->right)
					break;

				if((long double)time >= g->left && (long double)time <= g->right)
				{
					int dst_cpu = g->event_sets->sets[cpu].comm_events[comm_event].dst_cpu;

					long double screen_x = roundl(gtk_trace_x_to_screen(g, time));
					int dst_cpu_idx = multi_event_set_find_cpu_idx(g->event_sets, dst_cpu);

					int y1 = (cpu < dst_cpu) ? cpu : dst_cpu;
					int y2 = (cpu < dst_cpu) ? dst_cpu : cpu;

					if(!(lines_painted[(int)screen_x].y1 <= y1 && lines_painted[(int)screen_x].y2 >= y2)) {
						cairo_move_to(cr, screen_x+0.5, cpu*cpu_height + cpu_height/2);
						cairo_line_to(cr, screen_x+0.5, dst_cpu_idx*cpu_height + cpu_height/2);
						cairo_stroke(cr);
						num_events_drawn++;

						if(lines_painted[(int)screen_x].y1 > y1 && lines_painted[(int)screen_x].y2 >= y2) {
							lines_painted[(int)screen_x].y1 = y1;
						} else if(lines_painted[(int)screen_x].y1 <= y1 && lines_painted[(int)screen_x].y2 < y2) {
							lines_painted[(int)screen_x].y2 = y2;
						} else if((lines_painted[(int)screen_x].y1 > y1 && lines_painted[(int)screen_x].y2 < y2) ||
							  (y2 - y1 > lines_painted[(int)screen_x].y2 - lines_painted[(int)screen_x].y1))
						{
							lines_painted[(int)screen_x].y1 = y1;
							lines_painted[(int)screen_x].y2 = y2;
						}
					}
				}
			}
		}
	}

	printf("Comm events drawn: %d\n", num_events_drawn);
	cairo_reset_clip(cr);
}

void gtk_trace_paint_single_events(GtkTrace* g, cairo_t* cr)
{
	double cpu_height = gtk_trace_cpu_height(g);
	const char* event_chars[] = { "?", "?", "?", "C" };

	cairo_set_source_rgb(cr, 0, 1.0, 0);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 8);

	for(int cpu = 0; cpu < g->event_sets->num_sets; cpu++) {
		int last_ev_px = -1;
		int single_event = event_set_get_first_single_event_in_interval(&g->event_sets->sets[cpu], (g->left > 0) ? g->left : 0, g->right);

		if(single_event != -1) {
			for(; single_event < g->event_sets->sets[cpu].num_single_events; single_event++) {
				uint64_t time = g->event_sets->sets[cpu].single_events[single_event].time;
				int type = g->event_sets->sets[cpu].single_events[single_event].type;

				if(g->event_sets->sets[cpu].single_events[single_event].time > g->right)
					break;

				long double screen_x = roundl(gtk_trace_x_to_screen(g, time));
				if(last_ev_px < (int)(screen_x)) {
					last_ev_px = (int)(screen_x);

					cairo_move_to(cr, screen_x,  cpu*cpu_height + 3);
					cairo_line_to(cr, screen_x,  (cpu+1)*cpu_height - 3);
					cairo_stroke(cr);

					cairo_rectangle(cr, screen_x, cpu*cpu_height + 3, 4, 4);
					cairo_fill(cr);

					cairo_move_to(cr, screen_x+5,  (cpu+1)*cpu_height - 3);
					cairo_show_text(cr, event_chars[type]);
				}
			}
		}
	}
}

void gtk_trace_set_bounds(GtkWidget *widget, long double left, long double right)
{
	GtkTrace* g = GTK_TRACE(widget);
	g->left = left;
	g->right = right;
	gtk_trace_paint(widget);
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
		gtk_trace_paint(widget);
}

void gtk_trace_set_draw_comm(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_comm);
	g->draw_comm = val;

	if(needs_redraw)
		gtk_trace_paint(widget);
}

void gtk_trace_set_draw_single_events(GtkWidget *widget, int val)
{
	GtkTrace* g = GTK_TRACE(widget);
	int needs_redraw = (val != g->draw_single_events);
	g->draw_single_events = val;

	if(needs_redraw)
		gtk_trace_paint(widget);
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
		gtk_trace_paint(widget);
}

void gtk_trace_paint(GtkWidget *widget)
{
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
