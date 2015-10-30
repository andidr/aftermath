/**
 * Copyright (C) 2014 Quentin Bunel <quentin.bunel@gmail.com>
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

#include "multi_histogram_widget.h"

#if CAIRO_HAS_PDF_SURFACE
#include <cairo/cairo-pdf.h>
#endif

#if CAIRO_HAS_PNG_SURFACE
#include <cairo/cairo-png.h>
#endif

#if CAIRO_HAS_SVG_SURFACE
#include <cairo/cairo-svg.h>
#endif

gint gtk_multi_histogram_signals[GTK_MULTI_HISTOGRAM_MAX_SIGNALS];

void gtk_multi_histogram_destroy(GtkObject *object)
{
	GtkMultiHistogramClass *class;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_MULTI_HISTOGRAM(object));

	class = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(class)->destroy) {
		(* GTK_OBJECT_CLASS(class)->destroy)(object);
	}
}

GtkWidget* gtk_multi_histogram_new(struct multi_event_set* mes)
{
	GtkMultiHistogram *g = gtk_type_new(gtk_multi_histogram_get_type());
	g->mes = mes;

	return GTK_WIDGET(g);
}

GtkType gtk_multi_histogram_get_type(void)
{
	static GtkType gtk_multi_histogram_type = 0;

	if (!gtk_multi_histogram_type) {
		static const GtkTypeInfo gtk_multi_histogram_type_info = {
			"GtkMultiHistogram",
			sizeof(GtkMultiHistogram),
			sizeof(GtkMultiHistogramClass),
			(GtkClassInitFunc) gtk_multi_histogram_class_init,
			(GtkObjectInitFunc) gtk_multi_histogram_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		gtk_multi_histogram_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_multi_histogram_type_info);
	}

	return gtk_multi_histogram_type;
}

void gtk_multi_histogram_class_init(GtkMultiHistogramClass *class)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;

	widget_class = (GtkWidgetClass *) class;
	object_class = (GtkObjectClass *) class;

	widget_class->realize = gtk_multi_histogram_realize;
	widget_class->size_request = gtk_multi_histogram_size_request;
	widget_class->size_allocate = gtk_multi_histogram_size_allocate;
	widget_class->expose_event = gtk_multi_histogram_expose;

	object_class->destroy = gtk_multi_histogram_destroy;
}

void gtk_multi_histogram_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_MULTI_HISTOGRAM(widget));
	g_return_if_fail(requisition != NULL);

	requisition->height = 50;
	requisition->height = 50;
}

void gtk_multi_histogram_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_MULTI_HISTOGRAM(widget));
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

void gtk_multi_histogram_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_MULTI_HISTOGRAM(widget));

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

gboolean gtk_multi_histogram_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_MULTI_HISTOGRAM(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	gtk_multi_histogram_paint(widget);

	return FALSE;
}

void gtk_multi_histogram_init(GtkMultiHistogram *histogram)
{
	histogram->histograms = NULL;
}

void gtk_multi_histogram_paint_context(GtkMultiHistogram *h, cairo_t* cr)
{
	int init_x, init_y;
	int x, y;
	long double* values = NULL;
	struct histogram* curr_hist;

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, h->widget.allocation.width, h->widget.allocation.height);
	cairo_fill(cr);

	if(h->histograms) {
		if(!(values = (long double*) calloc(h->histograms->num_hist_bins, sizeof(long double))))
			return;

		values = memcpy(values, h->histograms->max_values, h->histograms->num_hist_bins*sizeof(long double));

		for(int hist_idx = 0; hist_idx < h->histograms->num_hists; hist_idx++) {
			init_x = 0;
			init_y = 0;
			x = 0;
			y = 0;

			curr_hist = h->histograms->histograms[hist_idx];

			double col_r = h->mes->tasks[h->histograms->task_ids[hist_idx]].color_r;
			double col_g = h->mes->tasks[h->histograms->task_ids[hist_idx]].color_g;
			double col_b = h->mes->tasks[h->histograms->task_ids[hist_idx]].color_b;

			cairo_set_source_rgb(cr, col_r, col_g, col_b);
			cairo_set_line_width(cr, 1.0);

			for(int i = 0; i < curr_hist->num_bins; i++) {
				x = (i * h->widget.allocation.width) / curr_hist->num_bins;
				y = (h->widget.allocation.height - (values[i] * h->widget.allocation.height));

				values[i] -= curr_hist->values[i];

				if(i == 0) {
					cairo_move_to(cr, x, y);
					init_x = x;
					init_y = y;
				} else
					cairo_line_to(cr, x, y);
			}

			cairo_line_to(cr, x, h->widget.allocation.height);
			cairo_line_to(cr, init_x, h->widget.allocation.height);
			cairo_line_to(cr, init_x, init_y);
			cairo_fill(cr);
		}

		free(values);
	}
}

void gtk_multi_histogram_paint(GtkWidget *widget)
{
	GtkMultiHistogram* h = GTK_MULTI_HISTOGRAM(widget);

	if(!gtk_widget_is_drawable(widget))
		return;

	cairo_t* cr = gdk_cairo_create(widget->window);
	gtk_multi_histogram_paint_context(h, cr);
	cairo_destroy(cr);
}

void gtk_multi_histogram_set_data(GtkWidget *widget, struct multi_histogram* d)
{
	GtkMultiHistogram* h = GTK_MULTI_HISTOGRAM(widget);
	h->histograms = d;

	printf("Setting histograms to %p\n", h->histograms);

	gtk_widget_queue_draw(widget);
}

int gtk_multi_histogram_save_to_file(GtkWidget *widget, enum export_file_format format, const char* filename)
{
	GtkMultiHistogram* h = GTK_MULTI_HISTOGRAM(widget);
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
	gtk_multi_histogram_paint_context(h, cr);
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
