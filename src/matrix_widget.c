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
#include "marshal.h"
#include <math.h>

#if CAIRO_HAS_PDF_SURFACE
#include <cairo/cairo-pdf.h>
#endif

#if CAIRO_HAS_PNG_SURFACE
#include <cairo/cairo-png.h>
#endif

#if CAIRO_HAS_SVG_SURFACE
#include <cairo/cairo-svg.h>
#endif

gint gtk_matrix_signals[GTK_MATRIX_MAX_SIGNALS];
gint gtk_matrix_motion_event(GtkWidget* widget, GdkEventMotion* event);

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
	g->min_threshold = 0.0;
	g->max_threshold = 1.0;

	g->last_x = -1;
	g->last_y = -1;

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

	gtk_matrix_signals[GTK_MATRIX_PAIR_UNDER_POINTER_CHANGED] =
                g_signal_new("pair-under-pointer-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(GtkMatrixClass, pair_changed),
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__INT_INT_INT64_DOUBLE,
                             G_TYPE_NONE, 4,
                             G_TYPE_INT, G_TYPE_INT, G_TYPE_INT64, G_TYPE_DOUBLE);

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

	requisition->width = 10;
	requisition->height = 10;
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
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(
		gtk_widget_get_parent_window (widget),
		& attributes, attributes_mask
		);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(gtk_matrix_motion_event), NULL);
}

gint gtk_matrix_motion_event(GtkWidget* widget, GdkEventMotion* event)
{
	GtkMatrix* h = GTK_MATRIX(widget);
	double width, height;

	if(h->matrix) {
		width = (double)widget->allocation.width / (double)h->matrix->width;
		height = (double)widget->allocation.height / (double)h->matrix->height;

		int node_x = event->x / width;
		int node_y = event->y / height;

		if(h->last_x != node_x || h->last_y != node_y)
			g_signal_emit(widget, gtk_matrix_signals[GTK_MATRIX_PAIR_UNDER_POINTER_CHANGED], 0,
				      node_x, node_y,
				      (int64_t)intensity_matrix_absolute_value_at(h->matrix, node_x, node_y),
				      (double)intensity_matrix_intensity_at(h->matrix, node_x, node_y));

		h->last_x = node_x;
		h->last_y = node_y;
	}

	return 0;
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

void gtk_matrix_paint_context(GtkMatrix *h, cairo_t* cr)
{
	double width, height;
	double threshold_range = h->max_threshold - h->min_threshold;

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_rectangle(cr, 0, 0, h->widget.allocation.width, h->widget.allocation.height);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
	cairo_set_line_width(cr, 1.0);

	if(h->matrix) {
		width = (double)h->widget.allocation.width / (double)h->matrix->width;
		height = (double)h->widget.allocation.height / (double)h->matrix->height;

		for(int x = 0; x < h->matrix->width; x++) {
			for(int y = 0; y < h->matrix->height; y++) {
				double intensity = intensity_matrix_intensity_at(h->matrix, x, y);
				intensity -= h->min_threshold;

				if(threshold_range != 0.0)
					intensity /= threshold_range;
				else
					intensity = 0.0;

				if(intensity > 1.0)
					intensity = 1.0;
				else if(intensity < 0.0)
					intensity = 0.0;

				cairo_rectangle(cr, x*width, y*height, width, height);
				cairo_set_source_rgb(cr, 1.0, 1.0-intensity, 1.0-intensity);
				cairo_fill(cr);
			}
		}

		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
		cairo_set_line_width(cr, 1.0);

		int draw_grid = (h->widget.allocation.width > 3*h->matrix->width) &&
			(h->widget.allocation.height > 3*h->matrix->height);


		if(draw_grid) {
			for(int x = 0; x < h->matrix->width; x++) {
				cairo_move_to(cr, floor(x*width)+0.5, 0);
				cairo_line_to(cr, floor(x*width)+0.5, h->widget.allocation.height);
				cairo_stroke(cr);
			}

			for(int y = 0; y < h->matrix->height; y++) {
				cairo_move_to(cr, 0, floor(y*height)+0.5);
				cairo_line_to(cr, h->widget.allocation.width, floor(y*height)+0.5);
				cairo_stroke(cr);
			}
		}

		cairo_rectangle(cr, 0, 0, h->widget.allocation.width-0.5, h->widget.allocation.height-0.5);
		cairo_stroke(cr);
	}
}

void gtk_matrix_paint(GtkWidget *widget)
{
	GtkMatrix* h = GTK_MATRIX(widget);

	if(!gtk_widget_is_drawable(widget))
		return;

	cairo_t* cr = gdk_cairo_create(widget->window);
	gtk_matrix_paint_context(h, cr);
	cairo_destroy(cr);
}

void gtk_matrix_set_data(GtkWidget *widget, struct intensity_matrix* m)
{
	GtkMatrix* gm = GTK_MATRIX(widget);
	gm->matrix = m;

	gtk_widget_queue_draw(widget);
}


void gtk_matrix_set_min_threshold(GtkWidget *widget, double val)
{
	GtkMatrix* g = GTK_MATRIX(widget);
	g->min_threshold = val;
	gtk_widget_queue_draw(widget);
}

void gtk_matrix_set_max_threshold(GtkWidget *widget, double val)
{
	GtkMatrix* g = GTK_MATRIX(widget);
	g->max_threshold = val;
	gtk_widget_queue_draw(widget);
}

int gtk_matrix_save_to_file(GtkWidget *widget, enum export_file_format format, const char* filename)
{
	GtkMatrix* m = GTK_MATRIX(widget);
	cairo_surface_t* surf = NULL;
	int err = 1;

	switch(format) {
		case EXPORT_FORMAT_PDF:
			#if CAIRO_HAS_PDF_SURFACE
			surf = cairo_pdf_surface_create(filename,
							widget->allocation.width,
							widget->allocation.height);
			#else
			goto out_err;
			#endif
			break;
		case EXPORT_FORMAT_SVG:
			#if CAIRO_HAS_SVG_SURFACE
			surf = cairo_svg_surface_create(filename,
							widget->allocation.width,
							widget->allocation.height);
			#else
			goto out_err;
			#endif
			break;
		case EXPORT_FORMAT_PNG:
			surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
							  widget->allocation.width,
							  widget->allocation.height);
			break;
	}

	if(cairo_surface_status(surf) == CAIRO_STATUS_NULL_POINTER)
		goto out_surf;

	cairo_t* cr = cairo_create(surf);
	gtk_matrix_paint_context(m, cr);
	cairo_destroy(cr);

	switch(format) {
		case EXPORT_FORMAT_PNG:
			if(cairo_surface_write_to_png(surf, filename) != CAIRO_STATUS_SUCCESS)
				goto out_surf;
			break;
		case EXPORT_FORMAT_PDF:
		case EXPORT_FORMAT_SVG:
			break;
	}

	err = 0;

out_surf:
	cairo_surface_destroy(surf);

	/* Suppresses warning about unused label */
	goto out_err;
out_err:
	return err;
}
