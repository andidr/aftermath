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
#include "histogram_widget.h"
#include "matrix_widget.h"
#include "globals.h"
#include "signals.h"
#include "dialogs.h"
#include "task_list.h"
#include "frame_list.h"
#include "numa_node_list.h"
#include "counter_list.h"
#include "debug.h"
#include "ansi_extras.h"
#include "util.h"
#include "multi_event_set.h"
#include "visuals_file.h"

struct load_thread_data {
	char* tracefile;
	off_t bytes_read;
	off_t trace_size;
	int error;
	uint64_t start_timestamp;
	struct progress_window_widgets* progress_widgets;
};

void* load_thread(void* pdata)
{
	struct load_thread_data* data = pdata;

	if(read_trace_sample_file(&g_mes, data->tracefile, &data->bytes_read) != 0) {
		data->error = 1;
		return NULL;
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

	if(ltd->bytes_read == ltd->trace_size || ltd->error) {
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
	char buffer[30];
	char title[PATH_MAX+10];

	g_visuals_filename = NULL;

	if(argc < 2 || argc > 4) {
		fprintf(stderr, "Usage: %s trace_file [executable [visuals]]\n", argv[0]);
		return 1;
	}

	gtk_init(&argc, &argv);
	tracefile = argv[1];

	if(argc > 2)
		executable = argv[2];

	if(argc > 3)
		g_visuals_filename = strdup(argv[3]);

	g_visuals_modified = 0;

	settings_init(&g_settings);

	if(read_user_settings(&g_settings) != 0) {
		show_error_message("Could not read settings");
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
		.bytes_read = 0,
		.error = 0,
		.trace_size = trace_size,
		.start_timestamp = ((uint64_t)tv.tv_sec)*1000000+tv.tv_usec,
		.progress_widgets = &progress_widgets,
	};

	pthread_create(&tid, NULL, load_thread, &load_thread_data);

	g_timeout_add(100, update_progress, &load_thread_data);

	gtk_main();

	pthread_join(tid, NULL);

	if(load_thread_data.error != 0) {
		show_error_message("Cannot read samples from %s", tracefile);
		return load_thread_data.error;
	}

	if(executable && debug_read_task_symbols(executable, &g_mes) != 0)
		show_error_message("Could not read debug symbols from %s", executable);

	if(g_visuals_filename) {
		if(load_visuals(g_visuals_filename, &g_mes) != 0)
			show_error_message("Could not read visuals from %s", g_visuals_filename);
	}

	xml = glade_xml_new(DATA_PATH "/ostv.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, toplevel_window);
	IMPORT_GLADE_WIDGET(xml, graph_box);
	IMPORT_GLADE_WIDGET(xml, hist_box);
	IMPORT_GLADE_WIDGET(xml, matrix_box);
	IMPORT_GLADE_WIDGET(xml, hscroll_bar);
	IMPORT_GLADE_WIDGET(xml, vscroll_bar);
	IMPORT_GLADE_WIDGET(xml, task_treeview);
	IMPORT_GLADE_WIDGET(xml, frame_treeview);
	IMPORT_GLADE_WIDGET(xml, frame_numa_node_treeview);
	IMPORT_GLADE_WIDGET(xml, counter_treeview);
	IMPORT_GLADE_WIDGET(xml, code_view);
	IMPORT_GLADE_WIDGET(xml, main_notebook);
	IMPORT_GLADE_WIDGET(xml, statusbar);
	IMPORT_GLADE_WIDGET(xml, selected_event_label);
	IMPORT_GLADE_WIDGET(xml, active_task_label);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_steals);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_pushes);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_data_reads);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_data_writes);
	IMPORT_GLADE_WIDGET(xml, toggle_tool_button_draw_size);
	IMPORT_GLADE_WIDGET(xml, use_global_values_check);
	IMPORT_GLADE_WIDGET(xml, global_values_min_entry);
	IMPORT_GLADE_WIDGET(xml, global_values_max_entry);
	IMPORT_GLADE_WIDGET(xml, use_global_slopes_check);
	IMPORT_GLADE_WIDGET(xml, global_slopes_min_entry);
	IMPORT_GLADE_WIDGET(xml, global_slopes_max_entry);

	IMPORT_GLADE_WIDGET(xml, label_perc_seeking);
	IMPORT_GLADE_WIDGET(xml, label_perc_texec);
	IMPORT_GLADE_WIDGET(xml, label_perc_tcreate);
	IMPORT_GLADE_WIDGET(xml, label_perc_resdep);
	IMPORT_GLADE_WIDGET(xml, label_perc_tdec);
	IMPORT_GLADE_WIDGET(xml, label_perc_bcast);
	IMPORT_GLADE_WIDGET(xml, label_perc_init);
	IMPORT_GLADE_WIDGET(xml, label_perc_estimate);
	IMPORT_GLADE_WIDGET(xml, label_perc_reorder);

	IMPORT_GLADE_WIDGET(xml, label_par_seeking);
	IMPORT_GLADE_WIDGET(xml, label_par_texec);
	IMPORT_GLADE_WIDGET(xml, label_par_tcreate);
	IMPORT_GLADE_WIDGET(xml, label_par_resdep);
	IMPORT_GLADE_WIDGET(xml, label_par_tdec);
	IMPORT_GLADE_WIDGET(xml, label_par_bcast);
	IMPORT_GLADE_WIDGET(xml, label_par_init);
	IMPORT_GLADE_WIDGET(xml, label_par_estimate);
	IMPORT_GLADE_WIDGET(xml, label_par_reorder);

	IMPORT_GLADE_WIDGET(xml, label_hist_selection_length);
	IMPORT_GLADE_WIDGET(xml, label_hist_avg_task_length);
	IMPORT_GLADE_WIDGET(xml, label_hist_num_tasks);
	IMPORT_GLADE_WIDGET(xml, label_hist_min_cycles);
	IMPORT_GLADE_WIDGET(xml, label_hist_max_cycles);
	IMPORT_GLADE_WIDGET(xml, label_hist_min_perc);
	IMPORT_GLADE_WIDGET(xml, label_hist_max_perc);

	IMPORT_GLADE_WIDGET(xml, use_task_length_check);
	IMPORT_GLADE_WIDGET(xml, task_length_min_entry);
	IMPORT_GLADE_WIDGET(xml, task_length_max_entry);

	IMPORT_GLADE_WIDGET(xml, use_comm_size_check);
	IMPORT_GLADE_WIDGET(xml, comm_size_min_entry);
	IMPORT_GLADE_WIDGET(xml, comm_size_max_entry);
	IMPORT_GLADE_WIDGET(xml, comm_numa_node_treeview);
	IMPORT_GLADE_WIDGET(xml, button_clear_range);
	IMPORT_GLADE_WIDGET(xml, label_range_selection);

	IMPORT_GLADE_WIDGET(xml, heatmap_min_cycles);
	IMPORT_GLADE_WIDGET(xml, heatmap_max_cycles);
	IMPORT_GLADE_WIDGET(xml, heatmap_num_shades);

	IMPORT_GLADE_WIDGET(xml, check_matrix_reads);
	IMPORT_GLADE_WIDGET(xml, check_matrix_writes);
	IMPORT_GLADE_WIDGET(xml, check_matrix_steals);
	IMPORT_GLADE_WIDGET(xml, check_matrix_pushes);
	IMPORT_GLADE_WIDGET(xml, check_matrix_reflexive);
	IMPORT_GLADE_WIDGET(xml, check_matrix_direction);
	IMPORT_GLADE_WIDGET(xml, check_matrix_numonly);
	IMPORT_GLADE_WIDGET(xml, label_comm_matrix);

	g_trace_widget = gtk_trace_new(&g_mes);
	gtk_container_add(GTK_CONTAINER(graph_box), g_trace_widget);

	g_histogram_widget = gtk_histogram_new();
	gtk_container_add(GTK_CONTAINER(hist_box), g_histogram_widget);

	g_matrix_widget = gtk_matrix_new();
	gtk_container_add(GTK_CONTAINER(matrix_box), g_matrix_widget);

	g_hscroll_bar = hscroll_bar;
	g_vscroll_bar = vscroll_bar;
	g_task_treeview = task_treeview;
	g_frame_treeview = frame_treeview;
	g_frame_numa_node_treeview = frame_numa_node_treeview;
	g_counter_treeview = counter_treeview;
	g_code_view = code_view;
	g_main_notebook = main_notebook;
	g_statusbar = statusbar;
	g_selected_event_label = selected_event_label;
	g_active_task_label = active_task_label;
	g_toggle_tool_button_draw_steals = toggle_tool_button_draw_steals;
	g_toggle_tool_button_draw_pushes = toggle_tool_button_draw_pushes;
	g_toggle_tool_button_draw_data_reads = toggle_tool_button_draw_data_reads;
	g_toggle_tool_button_draw_data_writes = toggle_tool_button_draw_data_writes;
	g_toggle_tool_button_draw_size = toggle_tool_button_draw_size;

	g_use_global_values_check = use_global_values_check;
	g_global_values_min_entry = global_values_min_entry;
	g_global_values_max_entry = global_values_max_entry;
	g_use_global_slopes_check = use_global_slopes_check;
	g_global_slopes_min_entry = global_slopes_min_entry;
	g_global_slopes_max_entry = global_slopes_max_entry;

	g_label_perc_seeking = label_perc_seeking;
	g_label_perc_texec = label_perc_texec;
	g_label_perc_tcreate = label_perc_tcreate;
	g_label_perc_resdep = label_perc_resdep;
	g_label_perc_tdec = label_perc_tdec;
	g_label_perc_bcast = label_perc_bcast;
	g_label_perc_init = label_perc_init;
	g_label_perc_estimate = label_perc_estimate;
	g_label_perc_reorder = label_perc_reorder;

	g_label_par_seeking = label_par_seeking;
	g_label_par_texec = label_par_texec;
	g_label_par_tcreate = label_par_tcreate;
	g_label_par_resdep = label_par_resdep;
	g_label_par_tdec = label_par_tdec;
	g_label_par_bcast = label_par_bcast;
	g_label_par_init = label_par_init;
	g_label_par_estimate = label_par_estimate;
	g_label_par_reorder = label_par_reorder;

	g_label_hist_selection_length = label_hist_selection_length;
	g_label_hist_avg_task_length = label_hist_avg_task_length;
	g_label_hist_num_tasks = label_hist_num_tasks;
	g_label_hist_min_cycles = label_hist_min_cycles;
	g_label_hist_max_cycles = label_hist_max_cycles;
	g_label_hist_min_perc = label_hist_min_perc;
	g_label_hist_max_perc = label_hist_max_perc;

	g_use_task_length_check = use_task_length_check;
	g_task_length_min_entry = task_length_min_entry;
	g_task_length_max_entry = task_length_max_entry;

	g_use_comm_size_check = use_comm_size_check;
	g_comm_size_min_entry = comm_size_min_entry;
	g_comm_size_max_entry = comm_size_max_entry;
	g_comm_numa_node_treeview = comm_numa_node_treeview;

	g_button_clear_range = button_clear_range;
	g_label_range_selection = label_range_selection;

	g_heatmap_min_cycles = heatmap_min_cycles;
	g_heatmap_max_cycles = heatmap_max_cycles;
	g_heatmap_num_shades = heatmap_num_shades;

	g_check_matrix_reads = check_matrix_reads;
	g_check_matrix_writes = check_matrix_writes;
	g_check_matrix_steals = check_matrix_steals;
	g_check_matrix_pushes = check_matrix_pushes;
	g_check_matrix_reflexive = check_matrix_reflexive;
	g_check_matrix_direction = check_matrix_direction;
	g_check_matrix_numonly = check_matrix_numonly;
	g_label_comm_matrix = label_comm_matrix;

	snprintf(buffer, sizeof(buffer), "%"PRId64, multi_event_get_min_counter_value(&g_mes));
	gtk_entry_set_text(GTK_ENTRY(g_global_values_min_entry), buffer);
	snprintf(buffer, sizeof(buffer), "%"PRId64, multi_event_get_max_counter_value(&g_mes));
	gtk_entry_set_text(GTK_ENTRY(g_global_values_max_entry), buffer);

	snprintf(buffer, sizeof(buffer), "%Lf", multi_event_get_min_counter_slope(&g_mes));
	gtk_entry_set_text(GTK_ENTRY(g_global_slopes_min_entry), buffer);
	snprintf(buffer, sizeof(buffer), "%Lf", multi_event_get_max_counter_slope(&g_mes));
	gtk_entry_set_text(GTK_ENTRY(g_global_slopes_max_entry), buffer);

	g_signal_connect(G_OBJECT(g_trace_widget), "bounds-changed", G_CALLBACK(trace_bounds_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "ybounds-changed", G_CALLBACK(trace_ybounds_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "state-event-under-pointer-changed", G_CALLBACK(trace_state_event_under_pointer_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "state-event-selection-changed", G_CALLBACK(trace_state_event_selection_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "range-selection-changed", G_CALLBACK(trace_range_selection_changed), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "create-annotation", G_CALLBACK(trace_create_annotation), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "edit-annotation", G_CALLBACK(trace_edit_annotation), g_trace_widget);
	g_signal_connect(G_OBJECT(toplevel_window), "delete-event", G_CALLBACK(check_quit), NULL);

	g_signal_connect(G_OBJECT(g_matrix_widget), "pair-under-pointer-changed", G_CALLBACK(comm_matrix_pair_under_pointer_changed), g_matrix_widget);

	task_list_init(GTK_TREE_VIEW(g_task_treeview));
	task_list_fill(GTK_TREE_VIEW(g_task_treeview), g_mes.tasks, g_mes.num_tasks);

	frame_list_init(GTK_TREE_VIEW(g_frame_treeview));
	frame_list_fill(GTK_TREE_VIEW(g_frame_treeview), g_mes.frames, g_mes.num_frames);

	numa_node_list_init(GTK_TREE_VIEW(g_frame_numa_node_treeview));
	numa_node_list_fill(GTK_TREE_VIEW(g_frame_numa_node_treeview), g_mes.max_numa_node_id);

	counter_list_init(GTK_TREE_VIEW(g_counter_treeview));
	counter_list_fill(GTK_TREE_VIEW(g_counter_treeview), g_mes.counters, g_mes.num_counters);

	numa_node_list_init(GTK_TREE_VIEW(g_comm_numa_node_treeview));
	numa_node_list_fill(GTK_TREE_VIEW(g_comm_numa_node_treeview), g_mes.max_numa_node_id);

	filter_init(&g_filter,
		    multi_event_get_min_counter_value(&g_mes),
		    multi_event_get_max_counter_value(&g_mes),
		    multi_event_get_min_counter_slope(&g_mes),
		    multi_event_get_max_counter_slope(&g_mes));

	if(histogram_init(&g_task_histogram, HISTOGRAM_DEFAULT_NUM_BINS, 0, 1)) {
		show_error_message("Cannot initialize task length histogram structure.");
		return 1;
	}

	if(intensity_matrix_init(&g_comm_matrix, 1, 1)) {
		show_error_message("Cannot initialize communication matrix.");
		return 1;
	}

	snprintf(title, sizeof(title), "OSTV - %s", tracefile);
	gtk_window_set_title(GTK_WINDOW(toplevel_window), title);

	gtk_widget_show_all(toplevel_window);

	reset_zoom();

	gtk_main();

	multi_event_set_destroy(&g_mes);
	filter_destroy(&g_filter);
	settings_destroy(&g_settings);
	histogram_destroy(&g_task_histogram);
	intensity_matrix_destroy(&g_comm_matrix);
	free(g_visuals_filename);

	return 0;
}
