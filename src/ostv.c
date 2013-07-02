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

#include <stdio.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <inttypes.h>
#include "glade_extras.h"
#include "trace_widget.h"
#include "paraver.h"
#include "globals.h"
#include "signals.h"
#include "detect.h"
#include "dialogs.h"
#include "task_list.h"
#include "debug.h"
#include "ansi_extras.h"
#include "util.h"
#include "multi_event_set.h"

struct load_thread_data {
	char* tracefile;
	enum trace_format format;
	off_t bytes_read;
	off_t trace_size;
	int error;
	uint64_t start_timestamp;
	struct progress_window_widgets* progress_widgets;
};

void* load_thread(void* pdata)
{
	struct load_thread_data* data = pdata;

	if(data->format == TRACE_FORMAT_OSTV) {
		if(read_trace_sample_file(&g_mes, data->tracefile, &data->bytes_read) != 0) {
			data->error = 1;
			return NULL;
		}
	} else if(data->format == TRACE_FORMAT_PARAVER) {
		if(read_paraver_samples(&g_mes, data->tracefile, &data->bytes_read) != 0) {
			data->error = 1;
			return NULL;
		}
	}

	return NULL;
}

gboolean update_progress(gpointer pdata)
{
	struct load_thread_data* ltd = pdata;
	char buffer[100];

	struct timeval tv;
	uint64_t timestamp;

	double seconds_elapsed;
	double throughput = 0;
	double seconds_remaining = 0;

	gettimeofday(&tv, NULL);
	timestamp = ((uint64_t)tv.tv_sec)*1000000+tv.tv_usec;

	pretty_print_bytes(buffer, sizeof(buffer), ltd->trace_size, "");
	gtk_label_set_text(ltd->progress_widgets->label_trace_bytes, buffer);

	pretty_print_bytes(buffer, sizeof(buffer), ltd->bytes_read, "");
	gtk_label_set_text(ltd->progress_widgets->label_bytes_loaded, buffer);

	seconds_elapsed = (double)(timestamp - ltd->start_timestamp) / 1000000.0;

	if(seconds_elapsed > 0) {
		throughput = (double)ltd->bytes_read / seconds_elapsed;
		seconds_remaining = (ltd->trace_size - ltd->bytes_read) / throughput;
	}

	pretty_print_bytes(buffer, sizeof(buffer), throughput, "/s");
	gtk_label_set_text(ltd->progress_widgets->label_throughput, buffer);

	snprintf(buffer, sizeof(buffer), "%.0fs", round(seconds_remaining));
	gtk_label_set_text(ltd->progress_widgets->label_seconds_remaining, buffer);

	gtk_progress_bar_set_fraction(ltd->progress_widgets->progressbar,
					(double)ltd->bytes_read / (double)ltd->trace_size);

	if(ltd->bytes_read == ltd->trace_size) {
		gtk_widget_hide(GTK_WIDGET(ltd->progress_widgets->window));
		gtk_main_quit();
		return FALSE;
	}

	return TRUE;
}

int main(int argc, char** argv)
{
	GladeXML *xml;
	char* tracefile = NULL;
	char* executable = NULL;
	enum trace_format format;

	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s trace_file [executable]\n", argv[0]);
		return 1;
	}

	gtk_init(&argc, &argv);
	tracefile = argv[1];

	if(argc > 2)
		executable = argv[2];

	settings_init(&g_settings);

	if(read_user_settings(&g_settings) != 0) {
		show_error_message("Could not read settings");
		return 1;
	}

	if(detect_trace_format(tracefile, &format)) {
		show_error_message("Could not open file %s", tracefile);
		return 1;
	}

	if(format == TRACE_FORMAT_UNKNOWN) {
		show_error_message("Cannot detect trace format of file %s", tracefile);
		return 1;
	}

	off_t trace_size = file_size(tracefile);

	if(trace_size == -1) {
		show_error_message("Cannot determine size of file %s", tracefile);
		return 1;
	}

	struct progress_window_widgets progress_widgets;
	pthread_t tid;
	struct timeval tv;

	show_progress_window_persistent(&progress_widgets);
	gettimeofday(&tv, NULL);

	struct load_thread_data load_thread_data = {
		.tracefile = tracefile,
		.format = format,
		.bytes_read = 0,
		.error = 0,
		.trace_size = trace_size,
		.start_timestamp = ((uint64_t)tv.tv_sec)*1000000+tv.tv_usec,
		.progress_widgets = &progress_widgets,
	};

	pthread_create(&tid, NULL, load_thread, &load_thread_data);

	g_timeout_add_seconds(1, update_progress, &load_thread_data);

	gtk_main();

	pthread_join(tid, NULL);

	if(load_thread_data.error != 0) {
		show_error_message("Cannot read samples from %s", tracefile);
		return load_thread_data.error;
	}

	if(executable && debug_read_task_symbols(executable, &g_mes) != 0)
		show_error_message("Could not read debug symbols from %s", executable);

	xml = glade_xml_new(DATA_PATH "/ostv.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, toplevel_window);
	IMPORT_GLADE_WIDGET(xml, graph_box);
	IMPORT_GLADE_WIDGET(xml, scroll_bar);
	IMPORT_GLADE_WIDGET(xml, task_treeview);
	IMPORT_GLADE_WIDGET(xml, code_view);
	IMPORT_GLADE_WIDGET(xml, main_notebook);
	IMPORT_GLADE_WIDGET(xml, statusbar);
	IMPORT_GLADE_WIDGET(xml, selected_event_label);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_steals);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_pushes);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_data_reads);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_size);

	g_trace_widget = gtk_trace_new(&g_mes);
	gtk_container_add(GTK_CONTAINER(graph_box), g_trace_widget);

	g_scroll_bar = scroll_bar;
	g_task_treeview = task_treeview;
	g_code_view = code_view;
	g_main_notebook = main_notebook;
	g_statusbar = statusbar;
	g_selected_event_label = selected_event_label;
	g_toggle_tool_button_draw_steals = toggle_tool_button_draw_steals;
	g_toggle_tool_button_draw_pushes = toggle_tool_button_draw_pushes;
	g_toggle_tool_button_draw_data_reads = toggle_tool_button_draw_data_reads;
	g_toggle_tool_button_draw_size = toggle_tool_button_draw_size;

	g_signal_connect(G_OBJECT(g_trace_widget), "bounds-changed", G_CALLBACK(trace_bounds_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "state-event-under-pointer-changed", G_CALLBACK(trace_state_event_under_pointer_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "state-event-selection-changed", G_CALLBACK(trace_state_event_selection_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_task_treeview), "row-activated", G_CALLBACK(task_treeview_row_activated), g_task_treeview);

	task_list_init(GTK_TREE_VIEW(g_task_treeview));
	task_list_fill(GTK_TREE_VIEW(g_task_treeview), g_mes.tasks, g_mes.num_tasks);

	filter_init(&g_filter);

	gtk_widget_show_all(toplevel_window);

	reset_zoom();

	gtk_main();

	multi_event_set_destroy(&g_mes);
	filter_destroy(&g_filter);
	settings_destroy(&g_settings);

	return 0;
}
