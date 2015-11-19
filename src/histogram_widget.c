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

#include <inttypes.h>
#include "marshal.h"
#include "histogram_widget.h"

#if CAIRO_HAS_PDF_SURFACE
#include <cairo/cairo-pdf.h>
#endif

#if CAIRO_HAS_PNG_SURFACE
#include <cairo/cairo-png.h>
#endif

#if CAIRO_HAS_SVG_SURFACE
#include <cairo/cairo-svg.h>
#endif

void gtk_histogram_emit_range_selection_changed(GtkWidget* widget, int64_t start, int64_t end);

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
	g->selection_enabled = 1;
	g->selecting = 0;

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
	widget_class->button_press_event = gtk_histogram_button_press_event;
	widget_class->button_release_event = gtk_histogram_button_release_event;

	gtk_histogram_signals[GTK_HISTOGRAM_RANGE_SELECTION_CHANGED] =
                g_signal_new("range-selection-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
			     0,
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__DOUBLE_DOUBLE,
                             G_TYPE_NONE, 2,
                             G_TYPE_DOUBLE, G_TYPE_DOUBLE);

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
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
				GDK_BUTTON_RELEASE_MASK | GDK_BUTTON1_MOTION_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(gtk_histogram_motion_event), NULL);

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

static inline long double gtk_histogram_pxpv_x(GtkHistogram* h)
{
	return (long double)h->widget.allocation.width  / (long double)(h->histogram->right - h->histogram->left);
}

static inline long double gtk_histogram_screen_x_to_histogram(GtkHistogram* h, int x)
{
	long double px_per_val = gtk_histogram_pxpv_x(h);
	return (x / px_per_val) + h->histogram->left;
}

static inline long double gtk_histogram_x_to_screen(GtkHistogram* h, long double x)
{
	long double px_per_val = gtk_histogram_pxpv_x(h);
	long double norm_val = x - h->histogram->left;
	long double norm_px_val = norm_val * px_per_val;
	long double norm_px_w_offs = norm_px_val;

	return norm_px_w_offs;
}

void gtk_histogram_paint_selection(GtkHistogram* h, cairo_t* cr)
{
	long double left_x = gtk_histogram_x_to_screen(h, h->range_selection_start);
	long double right_x = gtk_histogram_x_to_screen(h, h->range_selection_end);
	cairo_rectangle(cr, 0, 0, h->widget.allocation.width, h->widget.allocation.height);
	cairo_clip(cr);

	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
	cairo_rectangle(cr, left_x, 0, right_x - left_x, h->widget.allocation.height);
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 0.3);
	cairo_rectangle(cr, left_x, 0, right_x - left_x, h->widget.allocation.height);
	cairo_stroke(cr);

	cairo_reset_clip(cr);
}

void gtk_histogram_paint_context(GtkHistogram* h, cairo_t* cr)
{
	int init_x = 0, init_y = 0;
	int x = 0, y = 0;

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, h->widget.allocation.width, h->widget.allocation.height);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
	cairo_set_line_width(cr, 1.0);

	if(h->histogram) {
		for(int i = 0; i < h->histogram->num_bins; i++) {
			x = (i * h->widget.allocation.width) / h->histogram->num_bins;
			y = h->widget.allocation.height -
				(h->histogram->values[i] * h->widget.allocation.height);

			if(i == 0) {
				cairo_move_to(cr, x, y);
				init_x = x;
				init_y = y;
			} else {
				cairo_line_to(cr, x, y);
			}
		}

		cairo_line_to(cr, x, h->widget.allocation.height);
		cairo_line_to(cr, init_x, h->widget.allocation.height);
		cairo_line_to(cr, init_x, init_y);
		cairo_fill(cr);
	}

	if(h->selecting)
		gtk_histogram_paint_selection(h, cr);
}

void gtk_histogram_paint(GtkWidget *widget)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);

	if(!gtk_widget_is_drawable(widget))
		return;

	cairo_t* cr = gdk_cairo_create(widget->window);
	gtk_histogram_paint_context(h, cr);
	cairo_destroy(cr);
}

void gtk_histogram_set_data(GtkWidget *widget, struct histogram* d)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);
	h->histogram = d;

	printf("Setting histogram to %p\n", h->histogram);

	gtk_widget_queue_draw(widget);
}

int gtk_histogram_save_to_file(GtkWidget *widget, enum export_file_format format, const char* filename)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);
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
	gtk_histogram_paint_context(h, cr);
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

gint gtk_histogram_button_press_event(GtkWidget* widget, GdkEventButton *event)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);

	if(!h->selection_enabled || !h->histogram)
		return TRUE;

	if(event->button == 1) {
		/* left click */
		h->selecting = 1;
		h->range_selection_start = gtk_histogram_screen_x_to_histogram(h, event->x);
	}
	return TRUE;
}

gint gtk_histogram_button_release_event(GtkWidget* widget, GdkEventButton* event)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);

	if(!h->selection_enabled || !h->histogram)
		return TRUE;

	if(event->button == 1) {
		gtk_histogram_emit_range_selection_changed(widget, h->range_selection_start, h->range_selection_end);
		h->selecting = 0;
		gtk_widget_queue_draw(widget);
	}

	return TRUE;
}

gint gtk_histogram_motion_event(GtkWidget* widget, GdkEventMotion* event)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);

	if(!h->selection_enabled || !h->histogram)
		return TRUE;

	if(h->selecting) {
		h->range_selection_end = gtk_histogram_screen_x_to_histogram(h, event->x);
		gtk_widget_queue_draw(widget);
	}

	return TRUE;
}

void gtk_histogram_emit_range_selection_changed(GtkWidget* widget, int64_t start, int64_t end)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);
	int64_t tmp;

	if(start > end) {
		tmp = start;
		start = end;
		end = tmp;
	}

	if(start < h->histogram->left)
		start = h->histogram->left;

	if(end > h->histogram->right)
		end = h->histogram->right;

	g_signal_emit(widget, gtk_histogram_signals[GTK_HISTOGRAM_RANGE_SELECTION_CHANGED], 0,
		      (double)start, (double)end);
}

void gtk_histogram_enable_range_selection(GtkWidget *widget)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);
	h->selection_enabled = 1;
}

void gtk_histogram_disable_range_selection(GtkWidget *widget)
{
	GtkHistogram* h = GTK_HISTOGRAM(widget);
	h->selection_enabled = 0;
}
