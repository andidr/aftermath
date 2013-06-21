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
#include <gtk/gtk.h>
#include "globals.h"
#include "trace_widget.h"
#include <inttypes.h>

G_MODULE_EXPORT void toolbar_zoom100_clicked(GtkButton *button, gpointer data)
{
	uint64_t start = multi_event_set_first_event_start(&g_mes);
	uint64_t end = multi_event_set_last_event_end(&g_mes);

	printf("ZOOM TO %"PRIu64" %"PRIu64"\n", start, end);

	gtk_trace_set_bounds(g_trace_widget, start, end);
}

G_MODULE_EXPORT void toolbar_rewind_clicked(GtkButton *button, gpointer data)
{
	uint64_t start = multi_event_set_first_event_start(&g_mes);
	gtk_trace_set_left(g_trace_widget, start);
}

G_MODULE_EXPORT void toolbar_ffwd_clicked(GtkButton *button, gpointer data)
{
	uint64_t end = multi_event_set_last_event_end(&g_mes);
	gtk_trace_set_right(g_trace_widget, end);
}

G_MODULE_EXPORT void toolbar_draw_states_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_states(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_comm_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_comm(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_single_events_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_single_events(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}
