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

#include "matrix_widget.h"

gint gtk_matrix_signals[GTK_MATRIX_MAX_SIGNALS];

void gtk_matrix_destroy(GtkObject *object)
{
	GtkMatrixClass *class;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_MATRIX(object));

	class = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(class)->destroy) {
		(* GTK_OBJECT_CLASS(class)->destroy)(object);
	}
}

GtkWidget* gtk_matrix_new(void)
{
	GtkMatrix *g = gtk_type_new(gtk_matrix_get_type());

	g->matrix = NULL;

	return GTK_WIDGET(g);
}

GtkType gtk_matrix_get_type(void)
{
	static GtkType gtk_matrix_type = 0;

	if (!gtk_matrix_type) {
		static const GtkTypeInfo gtk_matrix_type_info = {
			"GtkMatrix",
			sizeof(GtkMatrix),
			sizeof(GtkMatrixClass),
			(GtkClassInitFunc) gtk_matrix_class_init,
			(GtkObjectInitFunc) gtk_matrix_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		gtk_matrix_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_matrix_type_info);
	}

	return gtk_matrix_type;
}

void gtk_matrix_class_init(GtkMatrixClass *class)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;

	widget_class = (GtkWidgetClass *) class;
	object_class = (GtkObjectClass *) class;

	widget_class->realize = gtk_matrix_realize;
	widget_class->size_request = gtk_matrix_size_request;
	widget_class->size_allocate = gtk_matrix_size_allocate;
	widget_class->expose_event = gtk_matrix_expose;

	object_class->destroy = gtk_matrix_destroy;
}

void gtk_matrix_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_MATRIX(widget));
	g_return_if_fail(requisition != NULL);

	requisition->height = 50;
	requisition->height = 50;
}

void gtk_matrix_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_MATRIX(widget));
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

void gtk_matrix_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_MATRIX(widget));

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

gboolean gtk_matrix_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_MATRIX(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	gtk_matrix_paint(widget);

	return FALSE;
}

void gtk_matrix_init(GtkMatrix *matrix)
{
}

void gtk_matrix_paint(GtkWidget *widget)
{
	double width, height;
	GtkMatrix* h = GTK_MATRIX(widget);

	if(!gtk_widget_is_drawable(widget))
		return;

	cairo_t* cr = gdk_cairo_create(widget->window);

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_rectangle(cr, 0, 0, widget->allocation.width, widget->allocation.height);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
	cairo_set_line_width(cr, 1.0);

	if(h->matrix) {
		width = (double)widget->allocation.width / (double)h->matrix->width;
		height = (double)widget->allocation.height / (double)h->matrix->height;

		for(int x = 0; x < h->matrix->width; x++) {
			for(int y = 0; y < h->matrix->height; y++) {
				double intensity = intensity_matrix_intensity_at(h->matrix, x, y);
				cairo_rectangle(cr, x*width, y*height, width, height);
				cairo_set_source_rgb(cr, 1.0, 1.0-intensity, 1.0-intensity);
				cairo_fill(cr);
			}
		}

		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
		cairo_set_line_width(cr, 1.0);

		for(int x = 0; x < h->matrix->width; x++) {
			cairo_move_to(cr, floor(x*width)+0.5, 0);
			cairo_line_to(cr, floor(x*width)+0.5, widget->allocation.height);
			cairo_stroke(cr);
		}

		for(int y = 0; y < h->matrix->height; y++) {
			cairo_move_to(cr, 0, floor(y*height)+0.5);
			cairo_line_to(cr, widget->allocation.width, floor(y*height)+0.5);
			cairo_stroke(cr);
		}

		cairo_rectangle(cr, 0, 0, widget->allocation.width-0.5, widget->allocation.height-0.5);
		cairo_stroke(cr);
	}

	cairo_destroy(cr);
}

void gtk_matrix_set_data(GtkWidget *widget, struct intensity_matrix* m)
{
	GtkMatrix* gm = GTK_MATRIX(widget);
	gm->matrix = m;

	gtk_widget_queue_draw(widget);
}
