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
#include "signals.h"
#include <gtk/gtk.h>
#include "globals.h"
#include "trace_widget.h"
#include <inttypes.h>
#include "dialogs.h"

G_MODULE_EXPORT void toolbar_zoom100_clicked(GtkButton *button, gpointer data)
{
	uint64_t start = multi_event_set_first_event_start(&g_mes);
	uint64_t end = multi_event_set_last_event_end(&g_mes);

	printf("ZOOM TO %"PRIu64" %"PRIu64"\n", start, end);

	gtk_trace_set_bounds(g_trace_widget, start, end);
	trace_bounds_changed(GTK_TRACE(g_trace_widget), (double)start, (double)end, NULL);
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

G_MODULE_EXPORT void menubar_double_buffering_toggled(GtkCheckMenuItem *item, gpointer data)
{
	gtk_trace_set_double_buffering(g_trace_widget, gtk_check_menu_item_get_active(item));
}

G_MODULE_EXPORT void menubar_goto_time(GtkMenuItem *item, gpointer data)
{
	double start = multi_event_set_first_event_start(&g_mes);
	double end = multi_event_set_last_event_end(&g_mes);
	long double left, right, new_left, new_right, range;
	double time;

	gtk_trace_get_bounds(g_trace_widget, &left, &right);
	range = right - left;

	if(show_goto_dialog(start, end, (double)((left+right)/2.0), &time)) {
		new_left = time - (range / 2);
		new_right = new_left + range;

		if(new_left < 0) {
			new_right -= new_left;
			new_left = 0;

			if(new_right > end)
				new_right = end;
		} else if(new_right > end) {
			new_left -= new_right - end;
			new_right = end;

			if(new_left < 0)
				new_left = 0;
		}

		gtk_trace_set_bounds(g_trace_widget, new_left, new_right);
		trace_bounds_changed(GTK_TRACE(g_trace_widget), new_left, new_right, NULL);
	}
}

static int react_to_scrollbar_change = 1;

G_MODULE_EXPORT void trace_bounds_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data)
{
	react_to_scrollbar_change = 0;
	double start = multi_event_set_first_event_start(&g_mes);
	double end = multi_event_set_last_event_end(&g_mes);
	double page_size = right-left;

	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(g_scroll_bar));

	gtk_adjustment_set_lower(adj, start + page_size / 2.0);
	gtk_adjustment_set_upper(adj, end + page_size / 2.0);
	gtk_adjustment_set_page_size(adj, page_size);
	gtk_adjustment_set_page_increment(adj, page_size);
	gtk_adjustment_set_step_increment(adj, page_size / 10.0);
	gtk_adjustment_set_value(adj, (left+right)/2.0);

	react_to_scrollbar_change = 1;
}

G_MODULE_EXPORT void scrollbar_value_changed(GtkHScrollbar *item, gdouble value, gpointer data)
{
	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(item));
	double page_size = gtk_adjustment_get_page_size(adj);

	if(react_to_scrollbar_change)
		gtk_trace_set_bounds(g_trace_widget, value - page_size / 2.0, value + page_size / 2.0);
}
