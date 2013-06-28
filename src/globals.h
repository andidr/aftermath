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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"
#include "settings.h"

extern GtkWidget* g_trace_widget;
extern GtkWidget* g_scroll_bar;
extern GtkWidget* g_task_treeview;
extern GtkWidget* g_code_view;
extern GtkWidget* g_main_notebook;
extern GtkWidget* g_statusbar;
extern GtkWidget* g_selected_event_label;
extern GtkWidget* g_toggle_tool_button_draw_steals;
extern GtkWidget* g_toggle_tool_button_draw_pushes;
extern GtkWidget* g_toggle_tool_button_draw_data_reads;
extern GtkWidget* g_toggle_tool_button_draw_size;
extern struct multi_event_set g_mes;
extern struct filter g_filter;
extern struct settings g_settings;

#endif
