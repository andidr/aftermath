#ifndef CELL_RENDERER_COLOR_BUTTON_H
#define CELL_RENDERER_COLOR_BUTTON_H

#include <gtk/gtk.h>

enum custom_cell_renderer_color_button_signals {
	CUSTOM_CELL_RENDERER_COLOR_BUTTON_COLOR_CHANGED = 0,
	CUSTOM_CELL_RENDERER_COLOR_BUTTON_MAX_SIGNALS
};

#define CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON                 (custom_cell_renderer_color_button_get_type())
#define CUSTOM_CELL_RENDERER_COLOR_BUTTON(obj)                 (G_TYPE_CHECK_INSTANCE_CAST((obj),  CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON, CustomCellRendererColorButton))
#define CUSTOM_CELL_RENDERER_COLOR_BUTTON_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),  CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON, CustomCellRendererColorButtonClass))
#define CUSTOM_IS_CELL_COLOR_BUTTON_COLOR_BUTTON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON))
#define CUSTOM_IS_CELL_COLOR_BUTTON_COLOR_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON))
#define CUSTOM_CELL_RENDERER_COLOR_BUTTON_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj),  CUSTOM_TYPE_CELL_RENDERER_COLOR_BUTTON, CustomCellRendererColorButtonClass))

typedef struct _CustomCellRendererColorButton CustomCellRendererColorButton;
typedef struct _CustomCellRendererColorButtonClass CustomCellRendererColorButtonClass;

struct _CustomCellRendererColorButton
{
	GtkCellRenderer parent;
	GdkColor color;
};


struct _CustomCellRendererColorButtonClass
{
	GtkCellRendererClass  parent_class;
	void (* color_changed)(CustomCellRendererColorButton* b);
};


GType custom_cell_renderer_color_button_get_type(void);
GtkCellRenderer* custom_cell_renderer_color_button_new(void);

#endif
