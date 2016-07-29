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
#include "multi_event_set.h"

GtkWidget* g_trace_widget;
GtkWidget* g_histogram_widget;
GtkWidget* g_multi_histogram_widget;
GtkWidget* g_histogram_widget_omp;
GtkWidget* g_counter_list_widget;
GtkWidget* g_matrix_widget;
GtkWidget* g_matrix_summary_widget;
GtkWidget* g_hscroll_bar;
GtkWidget* g_vscroll_bar;
GtkWidget* g_omp_for_treeview;
GtkWidget* g_omp_for_treeview_type;
GtkWidget* g_omp_task_treeview;
GtkWidget* g_omp_task_treeview_type;
GtkWidget* g_task_treeview;
GtkWidget* g_cpu_treeview;
GtkWidget* g_writes_to_numa_nodes_treeview;
GtkWidget* g_frame_treeview;
GtkWidget* g_frame_numa_node_treeview;
GtkWidget* g_counter_treeview;
GtkWidget* g_state_treeview;
GtkWidget* g_code_view;
GtkWidget* g_main_notebook;
GtkWidget* g_statusbar;
GtkWidget* g_task_selected_event_label;
GtkWidget* g_active_task_label;
GtkWidget* g_active_for_label;
GtkWidget* g_active_for_instance_label;
GtkWidget* g_active_for_chunk_label;
GtkWidget* g_active_for_chunk_part_label;
GtkWidget* g_openmp_task_selected_event_label;
GtkWidget* g_active_omp_task_label;
GtkWidget* g_toggle_tool_button_draw_steals;
GtkWidget* g_toggle_tool_button_draw_pushes;
GtkWidget* g_toggle_tool_button_draw_data_reads;
GtkWidget* g_toggle_tool_button_draw_data_writes;
GtkWidget* g_toggle_tool_button_draw_size;
GtkWidget* g_use_global_values_check;
GtkWidget* g_global_values_min_entry;
GtkWidget* g_global_values_max_entry;
GtkWidget* g_use_global_slopes_check;
GtkWidget* g_global_slopes_min_entry;
GtkWidget* g_global_slopes_max_entry;
GtkWidget* g_writes_to_numa_nodes_min_size_entry;
GtkWidget* g_toggle_omp_for;

GtkWidget* g_label_hist_num_tasks;
GtkWidget* g_label_hist_num_chunk_parts;
GtkWidget* g_label_hist_selection_length;
GtkWidget* g_label_hist_selection_length_omp;
GtkWidget* g_label_hist_avg_task_length;
GtkWidget* g_label_hist_avg_chunk_part_length;
GtkWidget* g_label_hist_num_tcreate;
GtkWidget* g_label_hist_min_cycles;
GtkWidget* g_label_hist_max_cycles;
GtkWidget* g_label_hist_min_perc;
GtkWidget* g_label_hist_max_perc;
GtkWidget* g_label_hist_min_cycles_omp;
GtkWidget* g_label_hist_max_cycles_omp;
GtkWidget* g_label_hist_min_perc_omp;
GtkWidget* g_label_hist_max_perc_omp;

GtkWidget* g_label_range_selection;
GtkWidget* g_label_range_selection_omp;
GtkWidget* g_button_clear_range;
GtkWidget* g_button_clear_range_omp;

GtkWidget* g_use_task_length_check;
GtkWidget* g_task_length_min_entry;
GtkWidget* g_task_length_max_entry;

GtkWidget* g_use_comm_size_check;
GtkWidget* g_comm_size_min_entry;
GtkWidget* g_comm_size_max_entry;
GtkWidget* g_comm_numa_node_treeview;

GtkWidget* g_heatmap_min_cycles;
GtkWidget* g_heatmap_max_cycles;
GtkWidget* g_heatmap_num_shades;

GtkWidget* g_check_matrix_reads;
GtkWidget* g_check_matrix_writes;
GtkWidget* g_check_matrix_steals;
GtkWidget* g_check_matrix_pushes;
GtkWidget* g_check_matrix_reflexive;
GtkWidget* g_check_matrix_direction;
GtkWidget* g_check_matrix_numonly;
GtkWidget* g_label_matrix_local_bytes;
GtkWidget* g_label_matrix_remote_bytes;
GtkWidget* g_label_matrix_local_perc;
GtkWidget* g_label_comm_matrix;

GtkWidget* g_check_single_c;
GtkWidget* g_check_single_es;
GtkWidget* g_check_single_ee;
GtkWidget* g_check_single_d;

GtkWidget* g_label_info_counter;
GtkWidget* g_global_hist_radio_button;
GtkWidget* g_per_task_hist_radio_button;
GtkWidget* g_counter_hist_radio_button;

GtkWidget* g_radio_matrix_mode_node;
GtkWidget* g_radio_matrix_mode_cpu;

GtkWidget* g_vbox_stats_all;
GtkWidget* g_vbox_comm;
GtkWidget* g_vbox_comm_pos;

struct multi_event_set g_mes;
struct filter g_filter;
struct settings g_settings;
struct histogram g_task_histogram;
struct multi_histogram g_task_multi_histogram;
struct histogram g_counter_histogram;
struct intensity_matrix g_comm_matrix;
struct intensity_matrix g_comm_summary_matrix;
struct trace_marker g_trace_markers[NUM_TRACE_MARKERS];
struct address_range_tree g_address_range_tree;
struct color_scheme_set g_color_scheme_set;
int g_address_range_tree_built;
enum gtk_trace_map_mode g_omp_map_mode;

int g_visuals_modified;
char* g_visuals_filename;

int g_draw_predecessors;
int g_predecessor_max_depth;
struct list_head* g_predecessors;
int* g_num_predecessors;
char* g_last_filter_expr;
