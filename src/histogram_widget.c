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

#include "histogram_widget.h"

gint gtk_histogram_signals[GTK_HISTOGRAM_MAX_SIGNALS];

void gtk_histogram_destroy(GtkObject *object)
{
	GtkHistogramClass *class;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_HISTOGRAM(object));


	class = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(class)->destroy) {
		(* GTK_OBJECT_CLASS(class)->destroy)(object);
	}
}

GtkWidget* gtk_histogram_new(void)
{
	GtkHistogram *g = gtk_type_new(gtk_histogram_get_type());

	g->histogram = NULL;

	return GTK_WIDGET(g);
}

GtkType gtk_histogram_get_type(void)
{
	static GtkType gtk_histogram_type = 0;

	if (!gtk_histogram_type) {
		static const GtkTypeInfo gtk_histogram_type_info = {
			"GtkHistogram",
			sizeof(GtkHistogram),
			sizeof(GtkHistogramClass),
			(GtkClassInitFunc) gtk_histogram_class_init,
			(GtkObjectInitFunc) gtk_histogram_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		gtk_histogram_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_histogram_type_info);
	}

	return gtk_histogram_type;
}

void gtk_histogram_class_init(GtkHistogramClass *class)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;

	widget_class = (GtkWidgetClass *) class;
	object_class = (GtkObjectClass *) class;

	widget_class->realize = gtk_histogram_realize;
	widget_class->size_request = gtk_histogram_size_request;
	widget_class->size_allocate = gtk_histogram_size_allocate;
	widget_class->expose_event = gtk_histogram_expose;

	object_class->destroy = gtk_histogram_destroy;
}

void gtk_histogram_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_HISTOGRAM(widget));
	g_return_if_fail(requisition != NULL);

	requisition->height = 50;
	requisition->height = 50;
}

void gtk_histogram_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_HISTOGRAM(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize(
			widget->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height
			);
	}
}

void gtk_histogram_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_HISTOGRAM(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(
		gtk_widget_get_parent_window (widget),
		& attributes, attributes_mask
		);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

gboolean gtk_histogram_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_HISTOGRAM(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	gtk_histogram_paint(widget);

	return FALSE;
}

void gtk_histogram_init(GtkHistogram *histogram)
{
}

void gtk_histogram_paint(GtkWidget *widget)
{
	int init_x = 0, init_y = 0;
	int x = 0, y = 0;
	GtkHistogram* h = GTK_HISTOGRAM(widget);

	if(!gtk_widget_is_drawable(widget))
		return;

	cairo_t* cr = gdk_cairo_create(widget->window);

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, widget->allocation.width, widget->allocation.height);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
	cairo_set_line_width(cr, 1.0);

	if(h->histogram) {
		for(int i = 0; i < h->histogram->num_bins; i++) {
			x = (i * widget->allocation.width) / h->histogram->num_bins;
			y = widget->allocation.height -
				(h->histogram->values[i] * widget->allocation.height);

			if(i == 0) {
				cairo_move_to(cr, x, y);
				init_x = x;
				init_y = y;
			} else {
				cairo_line_to(cr, x, y);
			}
		}

		cairo_line_to(cr, x, widget->allocation.height);
		cairo_line_to(cr, init_x, widget->allocation.height);
		cairo_line_to(cr, init_x, init_y);
		cairo_fill(cr);
	}

	cairo_destroy(cr);
}

void gtk_histogram_set_data(GtkWidget *widget, struct histogram* d)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);
	h->histogram = d;

	printf("Setting histogram to %p\n", h->histogram);

	gtk_widget_queue_draw(widget);
}
