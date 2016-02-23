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
#include "statistics.h"
#include "trace_widget.h"
#include "intensity_matrix.h"
#include "address_range_tree.h"
#include "contrib/linux-kernel/list.h"
#include "color_scheme.h"

extern GtkWidget* g_trace_widget;
extern GtkWidget* g_histogram_widget;
extern GtkWidget* g_multi_histogram_widget;
extern GtkWidget* g_counter_list_widget;
extern GtkWidget* g_matrix_widget;
extern GtkWidget* g_matrix_summary_widget;
extern GtkWidget* g_hscroll_bar;
extern GtkWidget* g_vscroll_bar;
extern GtkWidget* g_task_treeview;
extern GtkWidget* g_cpu_treeview;
extern GtkWidget* g_frame_treeview;
extern GtkWidget* g_writes_to_numa_nodes_treeview;
extern GtkWidget* g_frame_numa_node_treeview;
extern GtkWidget* g_counter_treeview;
extern GtkWidget* g_state_treeview;
extern GtkWidget* g_code_view;
extern GtkWidget* g_main_notebook;
extern GtkWidget* g_statusbar;
extern GtkWidget* g_selected_event_label;
extern GtkWidget* g_active_task_label;
extern GtkWidget* g_toggle_tool_button_draw_steals;
extern GtkWidget* g_toggle_tool_button_draw_pushes;
extern GtkWidget* g_toggle_tool_button_draw_data_reads;
extern GtkWidget* g_toggle_tool_button_draw_data_writes;
extern GtkWidget* g_toggle_tool_button_draw_size;
extern GtkWidget* g_writes_to_numa_nodes_min_size_entry;
extern GtkWidget* g_toggle_omp_for;

extern GtkWidget* g_use_global_values_check;
extern GtkWidget* g_global_values_min_entry;
extern GtkWidget* g_global_values_max_entry;

extern GtkWidget* g_use_global_slopes_check;
extern GtkWidget* g_global_slopes_min_entry;
extern GtkWidget* g_global_slopes_max_entry;

extern GtkWidget* g_label_hist_num_tasks;
extern GtkWidget* g_label_hist_selection_length;
extern GtkWidget* g_label_hist_avg_task_length;
extern GtkWidget* g_label_hist_num_tcreate;
extern GtkWidget* g_label_hist_min_cycles;
extern GtkWidget* g_label_hist_max_cycles;
extern GtkWidget* g_label_hist_min_perc;
extern GtkWidget* g_label_hist_max_perc;

extern GtkWidget* g_label_range_selection;
extern GtkWidget* g_button_clear_range;

extern GtkWidget* g_use_task_length_check;
extern GtkWidget* g_task_length_min_entry;
extern GtkWidget* g_task_length_max_entry;

extern GtkWidget* g_use_comm_size_check;
extern GtkWidget* g_comm_size_min_entry;
extern GtkWidget* g_comm_size_max_entry;
extern GtkWidget* g_comm_numa_node_treeview;

extern GtkWidget* g_heatmap_min_cycles;
extern GtkWidget* g_heatmap_max_cycles;
extern GtkWidget* g_heatmap_num_shades;

extern GtkWidget* g_check_matrix_reads;
extern GtkWidget* g_check_matrix_writes;
extern GtkWidget* g_check_matrix_steals;
extern GtkWidget* g_check_matrix_pushes;
extern GtkWidget* g_check_matrix_reflexive;
extern GtkWidget* g_check_matrix_direction;
extern GtkWidget* g_check_matrix_numonly;
extern GtkWidget* g_label_matrix_local_bytes;
extern GtkWidget* g_label_matrix_remote_bytes;
extern GtkWidget* g_label_matrix_local_perc;
extern GtkWidget* g_label_comm_matrix;

extern GtkWidget* g_check_single_c;
extern GtkWidget* g_check_single_es;
extern GtkWidget* g_check_single_ee;
extern GtkWidget* g_check_single_d;

extern GtkWidget* g_label_info_counter;
extern GtkWidget* g_global_hist_radio_button;
extern GtkWidget* g_per_task_hist_radio_button;
extern GtkWidget* g_counter_hist_radio_button;

extern GtkWidget* g_radio_matrix_mode_node;
extern GtkWidget* g_radio_matrix_mode_cpu;

extern GtkWidget* g_vbox_stats_all;
extern GtkWidget* g_vbox_comm;
extern GtkWidget* g_vbox_comm_pos;

extern struct multi_event_set g_mes;
extern struct filter g_filter;
extern struct settings g_settings;
extern struct histogram g_task_histogram;
extern struct histogram g_counter_histogram;
extern struct multi_histogram g_task_multi_histogram;
extern struct intensity_matrix g_comm_matrix;
extern struct intensity_matrix g_comm_summary_matrix;
extern struct address_range_tree g_address_range_tree;
extern int g_address_range_tree_built;
extern struct color_scheme_set g_color_scheme_set;
extern enum gtk_trace_map_mode g_omp_map_mode;

#define NUM_TRACE_MARKERS 6
extern struct trace_marker g_trace_markers[NUM_TRACE_MARKERS];

#define TCREATE_TRACE_MARKER_COLOR_R 0.0
#define TCREATE_TRACE_MARKER_COLOR_G 0.5
#define TCREATE_TRACE_MARKER_COLOR_B 0.0

#define TDESTROY_TRACE_MARKER_COLOR_R 1.0
#define TDESTROY_TRACE_MARKER_COLOR_G 0.0
#define TDESTROY_TRACE_MARKER_COLOR_B 0.0

#define FIRSTWRITE_TRACE_MARKER_COLOR_R 0.25
#define FIRSTWRITE_TRACE_MARKER_COLOR_G 0.25
#define FIRSTWRITE_TRACE_MARKER_COLOR_B 0.75

#define FIRSTMAXWRITE_TRACE_MARKER_COLOR_R 1.0
#define FIRSTMAXWRITE_TRACE_MARKER_COLOR_G 0.0
#define FIRSTMAXWRITE_TRACE_MARKER_COLOR_B 1.0

#define PREV_TCREATE_TRACE_MARKER_COLOR_R 0.0
#define PREV_TCREATE_TRACE_MARKER_COLOR_G 1.0
#define PREV_TCREATE_TRACE_MARKER_COLOR_B 0.0

#define READY_TRACE_MARKER_COLOR_R 1.0
#define READY_TRACE_MARKER_COLOR_G 0.5
#define READY_TRACE_MARKER_COLOR_B 0.0

extern int g_visuals_modified;
extern char* g_visuals_filename;

extern int g_draw_predecessors;
extern int g_predecessor_max_depth;
extern struct list_head* g_predecessors;
extern int* g_num_predecessors;

extern char* g_last_filter_expr;

#endif
