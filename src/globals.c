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

#include "globals.h"

GtkWidget* g_trace_widget;
GtkWidget* g_scroll_bar;
GtkWidget* g_task_treeview;
GtkWidget* g_code_view;
GtkWidget* g_main_notebook;
GtkWidget* g_statusbar;
GtkWidget* g_selected_event_label;
GtkWidget* g_toggle_tool_button_draw_steals;
GtkWidget* g_toggle_tool_button_draw_pushes;
GtkWidget* g_toggle_tool_button_draw_data_reads;

struct multi_event_set g_mes = { .sets = NULL,
				 .num_sets = 0,
				 .num_sets_free = 0,
				 .num_tasks = 0,
				 .num_tasks_free = 0,
				 .tasks = NULL};

struct filter g_filter;
