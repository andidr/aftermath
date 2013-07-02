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

#include "cell_renderer_color_button.h"
#include "dialogs.h"
#include <stdint.h>
#include <malloc.h>

static void custom_cell_renderer_color_button_init(CustomCellRendererColorButton* cellcolor_button);
static void custom_cell_renderer_color_button_class_init(CustomCellRendererColorButtonClass* klass);
static void custom_cell_renderer_color_button_get_property(GObject* object, guint param_id, GValue* value, GParamSpec* pspec);
static void custom_cell_renderer_color_button_set_property(GObject* object, guint param_id, const GValue* value, GParamSpec* pspec);
static void custom_cell_renderer_color_button_finalize(GObject* gobject);

static void custom_cell_renderer_color_button_get_size(GtkCellRenderer* cell, GtkWidget* widget, GdkRectangle* cell_area, gint* x_offset, gint* y_offset, gint* width, gint* height);
static void custom_cell_renderer_color_button_render(GtkCellRenderer* cell, GdkWindow* window, GtkWidget* widget, GdkRectangle* background_area, GdkRectangle* cell_area, GdkRectangle* expose_area, guint flags);

enum
{
	PROP_COLOR = 1,
};

static gpointer parent_class;
static gint custom_cell_renderer_color_button_signals[CUSTOM_CELL_RENDERER_COLOR_BUTTON_MAX_SIGNALS] = { 0 };

GType custom_cell_renderer_color_button_get_type(void)
{
	static GType cell_color_button_type = 0;

	if(cell_color_button_type == 0)
	{
		static const GTypeInfo cell_color_button_info =
			{
				sizeof(CustomCellRendererColorButtonClass),
				NULL, /* base_init */
				NULL, /* base_finalize */
				(GClassInitFunc) custom_cell_renderer_color_button_class_init,
				NULL, /* class_finalize */
				NULL, /* class_data */
				sizeof(CustomCellRendererColorButton),
				0, /* n_preallocs */
				(GInstanceInitFunc) custom_cell_renderer_color_button_init,
			};

		/* Derive from GtkCellRenderer */
		cell_color_button_type = g_type_register_static(GTK_TYPE_CELL_RENDERER, "CustomCellRendererColorButton", &cell_color_button_info, 0);
	}

	return cell_color_button_type;
}

static gboolean custom_cell_renderer_color_button_activate(GtkCellRenderer* cell, GdkEvent* event, GtkWidget* widget, const gchar* path,
							   const GdkRectangle* background_area, const GdkRectangle* cell_area,
							   GtkCellRendererState  flags)
{
	CustomCellRendererColorButton* cb = CUSTOM_CELL_RENDERER_COLOR_BUTTON(cell);
	GdkColor color = cb->color;

	if(show_color_dialog(&color))
		g_signal_emit(cell, custom_cell_renderer_color_button_signals[CUSTOM_CELL_RENDERER_COLOR_BUTTON_COLOR_CHANGED], 0, path, &color);

	return TRUE;
}

static void custom_cell_renderer_color_button_init(CustomCellRendererColorButton* cellrenderercolor_button)
{
	GTK_CELL_RENDERER(cellrenderercolor_button)->mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
	GTK_CELL_RENDERER(cellrenderercolor_button)->xpad = 2;
	GTK_CELL_RENDERER(cellrenderercolor_button)->ypad = 2;

	g_object_set(G_OBJECT(cellrenderercolor_button), "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
}

static void custom_cell_renderer_color_button_class_init(CustomCellRendererColorButtonClass* klass)
{
	GtkCellRendererClass* cell_class = GTK_CELL_RENDERER_CLASS(klass);
	GObjectClass* object_class = G_OBJECT_CLASS(klass);

	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = custom_cell_renderer_color_button_finalize;
	object_class->get_property = custom_cell_renderer_color_button_get_property;
	object_class->set_property = custom_cell_renderer_color_button_set_property;
	cell_class->activate = (void*)custom_cell_renderer_color_button_activate;
	cell_class->get_size = custom_cell_renderer_color_button_get_size;
	cell_class->render = custom_cell_renderer_color_button_render;
	g_object_class_install_property(object_class, PROP_COLOR, g_param_spec_boxed("color", "color", "The color to display", GDK_TYPE_COLOR, G_PARAM_READWRITE));

	custom_cell_renderer_color_button_signals[CUSTOM_CELL_RENDERER_COLOR_BUTTON_COLOR_CHANGED] =
                g_signal_new("color-changed", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(CustomCellRendererColorButtonClass, color_changed),
                             NULL, NULL,
                             gtk_marshal_VOID__POINTER_POINTER,
                             G_TYPE_NONE, 2,
                             G_TYPE_POINTER, G_TYPE_POINTER);
}


static void custom_cell_renderer_color_button_finalize(GObject* object)
{
	(* G_OBJECT_CLASS(parent_class)->finalize) (object);
}

static void custom_cell_renderer_color_button_get_property(GObject* object, guint param_id, GValue* value, GParamSpec* psec)
{
	CustomCellRendererColorButton* cellcolor_button = CUSTOM_CELL_RENDERER_COLOR_BUTTON(object);
	gpointer tmp;

	switch(param_id)
	{
		case PROP_COLOR:
			tmp = g_value_get_boxed(value);
			*((GdkColor*)tmp) = cellcolor_button->color;
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, psec);
			break;
	}
}

static void custom_cell_renderer_color_button_set_property(GObject* object, guint param_id, const GValue* value, GParamSpec* pspec)
{
	CustomCellRendererColorButton* cellcolor_button = CUSTOM_CELL_RENDERER_COLOR_BUTTON(object);
	gpointer tmp;

	switch(param_id)
	{
		case PROP_COLOR:
			tmp = g_value_get_boxed(value);
			cellcolor_button->color = *((GdkColor*)tmp);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
			break;
	}
}

GtkCellRenderer* custom_cell_renderer_color_button_new(void)
{
	return g_object_new(CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON, NULL);
}

#define FIXED_WIDTH 10
#define FIXED_HEIGHT 10

static void custom_cell_renderer_color_button_get_size(GtkCellRenderer* cell, GtkWidget* widget, GdkRectangle* cell_area, gint* x_offset, gint* y_offset, gint* width, gint* height)
{
	gint calc_width;
	gint calc_height;

	calc_width = (gint) cell->xpad * 2 + FIXED_WIDTH;
	calc_height = (gint) cell->ypad * 2 + FIXED_HEIGHT;

	if(width)
		*width = calc_width;

	if(height)
		*height = calc_height;

	if(cell_area)
	{
		if(x_offset)
		{
			*x_offset = cell->xalign * (cell_area->width - calc_width);
			*x_offset = MAX(*x_offset, 0);
		}

		if(y_offset)
		{
			*y_offset = cell->yalign * (cell_area->height - calc_height);
			*y_offset = MAX(*y_offset, 0);
		}
	}
}


static void custom_cell_renderer_color_button_render(GtkCellRenderer* cell, GdkWindow* window, GtkWidget* widget,
						     GdkRectangle* background_area, GdkRectangle* cell_area,
						     GdkRectangle* expose_area, guint flags)
{
	CustomCellRendererColorButton* cellcolor_button = CUSTOM_CELL_RENDERER_COLOR_BUTTON(cell);
	gint width, height;
	gint x_offset, y_offset;

	custom_cell_renderer_color_button_get_size(cell, widget, cell_area,
						   &x_offset, &y_offset,
						   &width, &height);

	cairo_t* cr = gdk_cairo_create(window);

	cairo_set_source_rgb(cr, cellcolor_button->color.red / 65535.0, cellcolor_button->color.green / 65535.0, cellcolor_button->color.blue / 65535.0);
	cairo_rectangle(cr, cell_area->x + x_offset + cell->xpad, cell_area->y + y_offset + cell->ypad, width - 2*cell->xpad, height - 2*cell->ypad);
	cairo_fill(cr);
	cairo_destroy(cr);
}
