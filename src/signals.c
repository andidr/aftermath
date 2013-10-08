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
#include "globals.h"
#include "trace_widget.h"
#include "histogram_widget.h"
#include "matrix_widget.h"
#include "util.h"
#include "dialogs.h"
#include "task_list.h"
#include "frame_list.h"
#include "numa_node_list.h"
#include "counter_list.h"
#include "ansi_extras.h"
#include "derived_counters.h"
#include "statistics.h"
#include "page.h"
#include "visuals_file.h"
#include "cpu_list.h"
#include <gtk/gtk.h>
#include <inttypes.h>
#include <stdio.h>

void reset_zoom(void)
{
	uint64_t start = multi_event_set_first_event_start(&g_mes);
	uint64_t end = multi_event_set_last_event_end(&g_mes);

	printf("ZOOM TO %"PRIu64" %"PRIu64"\n", start, end);

	gtk_trace_set_bounds(g_trace_widget, start, end);
	trace_bounds_changed(GTK_TRACE(g_trace_widget), (double)start, (double)end, NULL);
}

/**
 * Connected to the "toggled" signal of a checkbutton, widget_toggle()
 * enables / disables another widget specified as the data parameter.
 */
G_MODULE_EXPORT gint widget_toggle(gpointer data, GtkWidget* check)
{
	GtkWidget* dependent = data;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
		gtk_widget_set_sensitive(dependent, 1);
	else
		gtk_widget_set_sensitive(dependent, 0);

	return 0;
}

G_MODULE_EXPORT void toolbar_zoom100_clicked(GtkButton *button, gpointer data)
{
	reset_zoom();
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

G_MODULE_EXPORT void use_global_values_toggled(GtkToggleButton *button, gpointer data)
{
	widget_toggle(g_global_values_min_entry, GTK_WIDGET(button));
	widget_toggle(g_global_values_max_entry, GTK_WIDGET(button));
}

G_MODULE_EXPORT void use_global_slopes_toggled(GtkToggleButton *button, gpointer data)
{
	widget_toggle(g_global_slopes_min_entry, GTK_WIDGET(button));
	widget_toggle(g_global_slopes_max_entry, GTK_WIDGET(button));
}

G_MODULE_EXPORT void use_task_length_check_toggle(GtkToggleButton *button, gpointer data)
{
	widget_toggle(g_task_length_min_entry, GTK_WIDGET(button));
	widget_toggle(g_task_length_max_entry, GTK_WIDGET(button));
}

G_MODULE_EXPORT void use_comm_size_check_toggle(GtkToggleButton *button, gpointer data)
{
	widget_toggle(g_comm_size_min_entry, GTK_WIDGET(button));
	widget_toggle(g_comm_size_max_entry, GTK_WIDGET(button));
}

G_MODULE_EXPORT void toolbar_draw_states_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_states(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_comm_toggled(GtkToggleToolButton *button, gpointer data)
{
	gboolean new_state = gtk_toggle_tool_button_get_active(button);

	gtk_trace_set_draw_comm(g_trace_widget, new_state);

	gtk_widget_set_sensitive(g_toggle_tool_button_draw_steals, new_state);
	gtk_widget_set_sensitive(g_toggle_tool_button_draw_pushes, new_state);
	gtk_widget_set_sensitive(g_toggle_tool_button_draw_data_reads, new_state);
	gtk_widget_set_sensitive(g_toggle_tool_button_draw_data_writes, new_state);
	gtk_widget_set_sensitive(g_toggle_tool_button_draw_size, new_state);
}

G_MODULE_EXPORT void toolbar_draw_comm_size_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_comm_size(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_steals_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_steals(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_pushes_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_pushes(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_data_reads_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_data_reads(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_data_writes_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_data_writes(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_single_events_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_single_events(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_counters_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_counters(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

G_MODULE_EXPORT void toolbar_draw_annotations_toggled(GtkToggleToolButton *button, gpointer data)
{
	gtk_trace_set_draw_annotations(g_trace_widget, gtk_toggle_tool_button_get_active(button));
}

int task_length_heatmap_update_params(void)
{
	const char* txt;
	int64_t min_length;
	int64_t max_length;
	int num_shades;

	txt = gtk_entry_get_text(GTK_ENTRY(g_heatmap_min_cycles));
	if(sscanf(txt, "%"PRId64, &min_length) != 1) {
		show_error_message("\"%s\" is not a correct integer value.", txt);
		return 1;
	}

	txt = gtk_entry_get_text(GTK_ENTRY(g_heatmap_max_cycles));
	if(sscanf(txt, "%"PRId64, &max_length) != 1) {
		show_error_message("\"%s\" is not a correct integer value.", txt);
		return 1;
	}

	num_shades = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_heatmap_num_shades));
	gtk_trace_set_heatmap_params(g_trace_widget, num_shades, min_length, max_length);

	return 0;
}

G_MODULE_EXPORT void task_length_heatmap_update_params_clicked(GtkButton *button, gpointer data)
{
	task_length_heatmap_update_params();
}

G_MODULE_EXPORT void tool_button_use_task_length_heatmap_toggled(GtkToggleToolButton *button, gpointer data)
{
	int active = gtk_toggle_tool_button_get_active(button);

	if(active) {
		if(task_length_heatmap_update_params())
			gtk_toggle_tool_button_set_active(button, 0);
	}

	gtk_trace_set_heatmap_mode(g_trace_widget, active);
}

G_MODULE_EXPORT void menubar_double_buffering_toggled(GtkCheckMenuItem *item, gpointer data)
{
	gtk_trace_set_double_buffering(g_trace_widget, gtk_check_menu_item_get_active(item));
}

G_MODULE_EXPORT void menubar_about(GtkMenuItem *item, gpointer data)
{
	show_about_dialog();
}

G_MODULE_EXPORT void menubar_settings(GtkCheckMenuItem *item, gpointer data)
{
	if(show_settings_dialog(&g_settings)) {
		if(write_user_settings(&g_settings) != 0)
			show_error_message("Could not write settings");
	}
}

G_MODULE_EXPORT void menubar_add_derived_counter(GtkMenuItem *item, gpointer data)
{
	struct derived_counter_options opt;
	struct counter_description* cd;
	int err = 1;

	filter_init(&opt.task_filter, 0, 0, 0, 0, multi_event_get_max_cpu(&g_mes));
	bitvector_init(&opt.cpus, multi_event_get_max_cpu(&g_mes));

	if(show_derived_counter_dialog(&g_mes, &opt)) {
		switch(opt.type) {
			case DERIVED_COUNTER_PARALLELISM:
				err = derive_parallelism_counter(&g_mes, &cd, opt.name, opt.state, opt.num_samples, opt.cpu);
				break;
			case DERIVED_COUNTER_AGGREGATE:
				err = derive_aggregate_counter(&g_mes, &cd, opt.name, opt.counter_idx, opt.num_samples, opt.cpu);
				break;
			case DERIVED_COUNTER_NUMA_CONTENTION:
				err = derive_numa_contention_counter(&g_mes, &cd, opt.name, opt.numa_node, opt.data_direction, opt.contention_type, opt.contention_model, opt.exclude_node, opt.num_samples, opt.cpu);
				break;
			case DERIVED_COUNTER_RATIO:
				err = derive_ratio_counter(&g_mes, &cd, opt.name, opt.ratio_type, opt.counter_idx, opt.divcounter_idx, opt.num_samples, opt.cpu);
				break;
			case DERIVED_COUNTER_TASK_LENGTH:
				err = derive_task_length_counter(&g_mes, &cd, opt.name, &opt.cpus, &opt.task_filter, opt.num_samples, opt.cpu);
				break;
		}

		if(err)
			show_error_message("Could not create derived counter.");
		else
			counter_list_append(GTK_TREE_VIEW(g_counter_treeview), cd, FALSE);

		free(opt.name);
	}

	filter_destroy(&opt.task_filter);
	bitvector_destroy(&opt.cpus);
}

void goto_time(double time)
{
	long double left, right, new_left, new_right, range;
	double end = multi_event_set_last_event_end(&g_mes);

	gtk_trace_get_bounds(g_trace_widget, &left, &right);
	range = right - left;

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

G_MODULE_EXPORT void menubar_goto_time(GtkMenuItem *item, gpointer data)
{
	double time;
	long double left, right;

	double start = multi_event_set_first_event_start(&g_mes);
	double end = multi_event_set_last_event_end(&g_mes);

	gtk_trace_get_bounds(g_trace_widget, &left, &right);

	if(show_goto_dialog(start, end, (double)((left+right)/2.0), &time))
		goto_time(time);
}

static int react_to_hscrollbar_change = 1;

G_MODULE_EXPORT void trace_bounds_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data)
{
	react_to_hscrollbar_change = 0;
	double start = multi_event_set_first_event_start(&g_mes);
	double end = multi_event_set_last_event_end(&g_mes);
	double page_size = right-left;

	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(g_hscroll_bar));

	gtk_adjustment_set_lower(adj, start + page_size / 2.0);
	gtk_adjustment_set_upper(adj, end + page_size / 2.0);
	gtk_adjustment_set_page_size(adj, page_size);
	gtk_adjustment_set_page_increment(adj, page_size);
	gtk_adjustment_set_step_increment(adj, page_size / 10.0);
	gtk_adjustment_set_value(adj, (left+right)/2.0);

	react_to_hscrollbar_change = 1;
}

static int react_to_vscrollbar_change = 1;

G_MODULE_EXPORT void trace_ybounds_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data)
{
	react_to_vscrollbar_change = 0;
	double start = 0;
	double end = g_mes.num_sets;
	double page_size = right-left;

	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(g_vscroll_bar));

	gtk_adjustment_set_lower(adj, start + page_size / 2.0);
	gtk_adjustment_set_upper(adj, end + page_size / 2.0);
	gtk_adjustment_set_page_size(adj, page_size);
	gtk_adjustment_set_page_increment(adj, page_size);
	gtk_adjustment_set_step_increment(adj, page_size / 10.0);
	gtk_adjustment_set_value(adj, (left+right)/2.0);

	react_to_vscrollbar_change = 1;
}

G_MODULE_EXPORT void trace_state_event_under_pointer_changed(GtkTrace* item, gpointer pstate_event, int cpu, int worker, gpointer data)
{
	static int message_id = -1;
	guint context_id = 0;
	char buffer[256];
	char buf_duration[40];
	struct state_event* se = pstate_event;

	if(message_id != -1)
		gtk_statusbar_remove(GTK_STATUSBAR(g_statusbar), context_id, message_id);

	if(se) {
		pretty_print_cycles(buf_duration, sizeof(buf_duration), se->end - se->start);

		snprintf(buffer, sizeof(buffer), "CPU %d: state %d (%s) from %"PRIu64" to %"PRIu64", duration: %scycles, active task: 0x%"PRIx64" %s",
			 cpu, se->state, worker_state_names[se->state], se->start, se->end, buf_duration, se->active_task->addr, se->active_task->symbol_name);

		message_id = gtk_statusbar_push(GTK_STATUSBAR(g_statusbar), context_id, buffer);
	}
}

G_MODULE_EXPORT void select_range_from_graph_button_clicked(GtkMenuItem *item, gpointer data)
{
	gtk_trace_enter_range_selection_mode(g_trace_widget);
}

G_MODULE_EXPORT void clear_range_button_clicked(GtkMenuItem *item, gpointer data)
{
	gtk_widget_set_sensitive(g_button_clear_range, FALSE);
	gtk_trace_clear_range_selection(g_trace_widget);
	gtk_label_set_markup(GTK_LABEL(g_label_range_selection), "<b>No range selected</b>");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_seeking), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_texec), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_tcreate), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_resdep), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_tdec), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_bcast), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_init), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_estimate), "");
	gtk_label_set_markup(GTK_LABEL(g_label_perc_reorder), "");

	gtk_label_set_markup(GTK_LABEL(g_label_par_seeking), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_texec), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_tcreate), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_resdep), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_tdec), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_bcast), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_init), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_estimate), "");
	gtk_label_set_markup(GTK_LABEL(g_label_par_reorder), "");

	gtk_label_set_text(GTK_LABEL(g_label_hist_num_tasks), "0 tasks considered");
	gtk_label_set_text(GTK_LABEL(g_label_hist_selection_length), "0 cycles");
	gtk_label_set_text(GTK_LABEL(g_label_hist_avg_task_length), "0 cycles / task (avg)");
	gtk_label_set_text(GTK_LABEL(g_label_hist_min_cycles), "0");
	gtk_label_set_text(GTK_LABEL(g_label_hist_max_cycles), "MAX");
	gtk_label_set_text(GTK_LABEL(g_label_hist_min_perc), "0");
	gtk_label_set_text(GTK_LABEL(g_label_hist_max_perc), "MAX");

	gtk_label_set_text(GTK_LABEL(g_label_comm_matrix), "\n");

	gtk_histogram_set_data(g_histogram_widget, NULL);
	gtk_matrix_set_data(g_matrix_widget, NULL);
}

void update_comm_matrix(void)
{
	int comm_mask = 0;
	int exclude_reflexive = 0;
	int ignore_direction = 0;
	int num_only = 0;
	int64_t left, right;

	if(!gtk_trace_get_range_selection(g_trace_widget, &left, &right))
		return;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_reads)))
		comm_mask |= 1 << COMM_TYPE_DATA_READ;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_writes)))
		comm_mask |= 1 << COMM_TYPE_DATA_WRITE;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_steals)))
		comm_mask |= 1 << COMM_TYPE_STEAL;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_pushes)))
		comm_mask |= 1 << COMM_TYPE_PUSH;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_reflexive)))
		exclude_reflexive = 1;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_direction)))
		ignore_direction = 1;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_numonly)))
		num_only = 1;

	intensity_matrix_destroy(&g_comm_matrix);

	if(numa_node_exchange_matrix_gather(&g_mes, &g_filter, &g_comm_matrix, left, right, comm_mask, exclude_reflexive, ignore_direction, num_only)) {
		show_error_message("Cannot gather communication matrix statistics.");
		return;
	}

	gtk_matrix_set_data(g_matrix_widget, &g_comm_matrix);
}

G_MODULE_EXPORT gint comm_matrix_reflexive_toggled(gpointer data, GtkWidget* check)
{
	update_comm_matrix();
	return 0;
}

G_MODULE_EXPORT void comm_matrix_min_threshold_changed(GtkRange* item, gpointer data)
{
	gtk_matrix_set_min_threshold(g_matrix_widget, gtk_range_get_value(item));
}

G_MODULE_EXPORT void comm_matrix_max_threshold_changed(GtkRange* item, gpointer data)
{
	gtk_matrix_set_max_threshold(g_matrix_widget, gtk_range_get_value(item));
}

G_MODULE_EXPORT gint comm_matrix_comm_type_toggled(gpointer data, GtkWidget* check)
{
	update_comm_matrix();
	return 0;
}

G_MODULE_EXPORT gint comm_matrix_direction_toggled(gpointer data, GtkWidget* check)
{
	update_comm_matrix();
	return 0;
}

G_MODULE_EXPORT gint comm_matrix_numonly_toggled(gpointer data, GtkWidget* check)
{
	update_comm_matrix();
	return 0;
}

void update_statistics(void)
{
	char buffer[128];
	char buffer2[128];
	struct state_statistics sts;
	struct task_statistics ts;
	int64_t left, right;
	int64_t length;

	if(!gtk_trace_get_range_selection(g_trace_widget, &left, &right))
		return;

	length = right - left;

	gtk_widget_set_sensitive(g_button_clear_range, TRUE);

	snprintf(buffer, sizeof(buffer), "<b>%"PRId64" - %"PRId64"</b>", left, right);
	gtk_label_set_markup(GTK_LABEL(g_label_range_selection), buffer);

	pretty_print_cycles(buffer, sizeof(buffer), right - left);
	snprintf(buffer2, sizeof(buffer2), "%scycles selected", buffer);
	gtk_label_set_text(GTK_LABEL(g_label_hist_selection_length), buffer2);

	if(left < 0)
		left = 0;

	if(right < 0)
		right = 0;

	state_statistics_init(&sts);
	state_statistics_gather_cycles(&g_mes, &g_filter, &sts, left, right);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_SEEKING]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_seeking), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_TASKEXEC]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_texec), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_TCREATE]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_tcreate), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_RESDEP]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_resdep), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_TDEC]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_tdec), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_BCAST]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_bcast), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_INIT]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_init), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_ESTIMATE_COSTS]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_estimate), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f%%", (100*(double)sts.state_cycles[WORKER_STATE_RT_REORDER]) / (double)(length*g_mes.num_sets));
	gtk_label_set_text(GTK_LABEL(g_label_perc_reorder), buffer);


	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_SEEKING] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_seeking), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_TASKEXEC] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_texec), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_TCREATE] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_tcreate), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_RESDEP] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_resdep), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_TDEC] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_tdec), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_BCAST] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_bcast), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_INIT] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_init), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_ESTIMATE_COSTS] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_estimate), buffer);

	snprintf(buffer, sizeof(buffer), "%.2f", (double)sts.state_cycles[WORKER_STATE_RT_REORDER] / (double)length);
	gtk_label_set_text(GTK_LABEL(g_label_par_reorder), buffer);

	if(task_statistics_init(&ts, HISTOGRAM_DEFAULT_NUM_BINS) != 0) {
		show_error_message("Cannot allocate task statistics structure.");
		return;
	}

	task_statistics_gather(&g_mes, &g_filter, &ts, left, right);

	if(ts.num_tasks > 0) {
		pretty_print_cycles(buffer, sizeof(buffer), ts.cycles / ts.num_tasks);
		snprintf(buffer2, sizeof(buffer2), "%scycles / task (avg)", buffer);
		gtk_label_set_text(GTK_LABEL(g_label_hist_avg_task_length), buffer2);
	}

	task_statistics_to_task_length_histogram(&ts, &g_task_histogram);
	gtk_histogram_set_data(g_histogram_widget, &g_task_histogram);

	snprintf(buffer, sizeof(buffer), "%d tasks considered", ts.num_tasks);
	gtk_label_set_text(GTK_LABEL(g_label_hist_num_tasks), buffer);

	pretty_print_cycles(buffer, sizeof(buffer), ts.min_cycles);
	gtk_label_set_text(GTK_LABEL(g_label_hist_min_cycles), buffer);

	pretty_print_cycles(buffer, sizeof(buffer), ts.max_cycles);
	gtk_label_set_text(GTK_LABEL(g_label_hist_max_cycles), buffer);

	gtk_label_set_text(GTK_LABEL(g_label_hist_min_perc), "0%");

	snprintf(buffer, sizeof(buffer), "%.2f%%", 100.0*((double)ts.max_hist / (double)ts.num_tasks));
	gtk_label_set_text(GTK_LABEL(g_label_hist_max_perc), buffer);

	task_statistics_destroy(&ts);

	update_comm_matrix();
}

G_MODULE_EXPORT void trace_range_selection_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data)
{
	update_statistics();
}

G_MODULE_EXPORT void trace_create_annotation(GtkTrace *item, int cpu, gdouble time)
{
	struct annotation a;
	struct event_set* es = multi_event_set_find_cpu(&g_mes, cpu);

	annotation_init(&a, cpu, time, "");

	if(show_annotation_dialog(&a, 0) == ANNOTATION_DIALOG_RESPONSE_OK) {
		event_set_add_annotation(es, &a);
		gtk_widget_queue_draw(g_trace_widget);

		g_visuals_modified = 1;
	}

	annotation_destroy(&a);
}

G_MODULE_EXPORT void trace_edit_annotation(GtkTrace *item, struct annotation* a)
{
	switch(show_annotation_dialog(a, 1)) {
		case ANNOTATION_DIALOG_RESPONSE_CANCEL:
			break;
		case ANNOTATION_DIALOG_RESPONSE_DELETE:
			event_set_delete_annotation(a->event_set, a);
		case ANNOTATION_DIALOG_RESPONSE_OK:
			gtk_widget_queue_draw(g_trace_widget);
			g_visuals_modified = 1;
			break;
	}
}

void task_link_activated(uint64_t addr)
{
	struct task* t = multi_event_set_find_task_by_addr(&g_mes, addr);

	if(t && t->source_filename)
		show_task_code(t);
}

void time_link_activated(uint64_t time)
{
	goto_time(time);
}

G_MODULE_EXPORT gint link_activated(GtkLabel *label, gchar *uri, gpointer user_data)
{
	uint64_t work_fn;
	uint64_t time;

	if(strstr(uri, "task://") == uri) {
		sscanf(uri, "task://0x%"PRIx64"", &work_fn);
		task_link_activated(work_fn);
	} else if(strstr(uri, "time://") == uri) {
		sscanf(uri, "time://%"PRIu64"", &time);
		time_link_activated(time);
	}

	return 1;
}

G_MODULE_EXPORT void trace_state_event_selection_changed(GtkTrace* item, gpointer pstate_event, int cpu, int worker, gpointer data)
{
	struct state_event* se = pstate_event;
	char buffer[4096];
	char buf_duration[40];
	char buf_tcreate[128];
	char buf_first_writer[128];
	char buf_first_max_writer[128];
	char buf_first_texec_start[128];
	char production_info[1024];
	char consumption_info[1024];
	char consumer_info[1024];

	uint64_t task_length;
	int valid;
	int num_markers = 0;

	if(se) {
		pretty_print_cycles(buf_duration, sizeof(buf_duration), se->end - se->start);

		snprintf(buffer, sizeof(buffer),
			 "CPU:\t\t%d\n"
			 "State\t\t%d (%s)\n"
			 "From\t\t<a href=\"time://%"PRIu64"\">%"PRIu64"</a> to <a href=\"time://%"PRIu64"\">%"PRIu64"</a>\n"
			 "Duration:\t%scycles\n",
			 cpu,
			 se->state,
			 worker_state_names[se->state],
			 se->start,
			 se->start,
			 se->end,
			 se->end,
			 buf_duration);

		gtk_label_set_markup(GTK_LABEL(g_selected_event_label), buffer);

		task_length = task_length_of_active_frame(se, &valid);
		if(valid) {
			pretty_print_cycles(buf_duration, sizeof(buf_duration), task_length);

			snprintf(buf_tcreate, sizeof(buf_tcreate),
				 "CPU %d at  <a href=\"time://%"PRIu64"\">%"PRIu64" cycles</a>, %"PRId32" bytes",
				 se->active_frame->first_tcreate->event_set->cpu,
				 se->active_frame->first_tcreate->time,
				 se->active_frame->first_tcreate->time,
				 se->active_frame->size);

			g_trace_markers[num_markers].time = se->active_frame->first_tcreate->time;
			g_trace_markers[num_markers].cpu = se->active_frame->first_tcreate->event_set->cpu;
			g_trace_markers[num_markers].color_r = TCREATE_TRACE_MARKER_COLOR_R;
			g_trace_markers[num_markers].color_g = TCREATE_TRACE_MARKER_COLOR_G;
			g_trace_markers[num_markers].color_b = TCREATE_TRACE_MARKER_COLOR_B;
			num_markers++;


			if(se->active_frame->first_write) {
				snprintf(buf_first_writer, sizeof(buf_first_writer),
					 "CPU %d at <a href=\"time://%"PRIu64"\">%"PRIu64" cycles</a>, %d bytes",
					 se->active_frame->first_write->event_set->cpu,
					 se->active_frame->first_write->time,
					 se->active_frame->first_write->time,
					 se->active_frame->first_write->size);

				g_trace_markers[num_markers].time = se->active_frame->first_write->time;
				g_trace_markers[num_markers].cpu = se->active_frame->first_write->event_set->cpu;
				g_trace_markers[num_markers].color_r = FIRSTWRITE_TRACE_MARKER_COLOR_R;
				g_trace_markers[num_markers].color_g = FIRSTWRITE_TRACE_MARKER_COLOR_G;
				g_trace_markers[num_markers].color_b = FIRSTWRITE_TRACE_MARKER_COLOR_B;
				num_markers++;
			} else {
				strncpy(buf_first_writer, "Task has no input data", sizeof(buf_first_writer));
			}

			if(se->active_frame->first_max_write) {
				snprintf(buf_first_max_writer, sizeof(buf_first_max_writer),
					 "CPU %d at <a href=\"time://%"PRIu64"\">%"PRIu64" cycles</a>, %d bytes",
					 se->active_frame->first_max_write->event_set->cpu,
					 se->active_frame->first_max_write->time,
					 se->active_frame->first_max_write->time,
					 se->active_frame->first_max_write->size);

				g_trace_markers[num_markers].time = se->active_frame->first_max_write->time;
				g_trace_markers[num_markers].cpu = se->active_frame->first_max_write->event_set->cpu;
				g_trace_markers[num_markers].color_r = FIRSTMAXWRITE_TRACE_MARKER_COLOR_R;
				g_trace_markers[num_markers].color_g = FIRSTMAXWRITE_TRACE_MARKER_COLOR_G;
				g_trace_markers[num_markers].color_b = FIRSTMAXWRITE_TRACE_MARKER_COLOR_B;
				num_markers++;
			} else {
				strncpy(buf_first_max_writer, "Task has no input data", sizeof(buf_first_writer));
			}

			if(se->active_frame->first_texec_start) {
				snprintf(buf_first_texec_start, sizeof(buf_first_texec_start),
					 "Node %d",
					 se->active_frame->numa_node);
			} else {
				strncpy(buf_first_texec_start, "Task never executed", sizeof(buf_first_texec_start));
			}

			int consumer_info_offs = 0;
			consumer_info[0] = '\0';

			int production_info_offs = 0;
			production_info[0] = '\0';

			int consumption_info_offs = 0;
			consumption_info[0] = '\0';

			struct comm_event* ce;
			struct single_event* cons_texec_start;
			int has_consumers = 0;
			for_each_comm_event_in_interval(se->event_set,
							se->texec_start->time,
							se->texec_start->next_texec_end->time,
							ce)
			{
				if(ce->type == COMM_TYPE_DATA_WRITE) {
					snprintf(production_info+production_info_offs,
							 sizeof(production_info)-production_info_offs-1,
							 "Node %d, %d bytes, <a href=\"time://%"PRIu64"\">%"PRIu64" cycles</a>\n",
							 ce->what->numa_node,
							 ce->size,
							 ce->time,
							 ce->time);
					production_info_offs += strlen(production_info+production_info_offs);

					if((cons_texec_start = multi_event_set_find_next_texec_start_for_frame(&g_mes, ce->time, ce->what))) {
						snprintf(consumer_info+consumer_info_offs,
							 sizeof(consumer_info)-consumer_info_offs-1,
							 "CPU %d, %d bytes, <a href=\"time://%"PRIu64"\">%"PRIu64" cycles</a>\n",
							 cons_texec_start->event_set->cpu,
							 ce->size,
							 cons_texec_start->time,
							 cons_texec_start->time);

						consumer_info_offs += strlen(consumer_info+consumer_info_offs);
						has_consumers = 1;
					}
				} else if(ce->type == COMM_TYPE_DATA_READ) {
					snprintf(consumption_info+consumption_info_offs,
							 sizeof(consumption_info)-consumption_info_offs-1,
							 "Node %d, %d bytes, <a href=\"time://%"PRIu64"\">%"PRIu64" cycles</a>\n",
							 ce->what->numa_node,
							 ce->size,
							 ce->time,
							 ce->time);
					consumption_info_offs += strlen(consumption_info+consumption_info_offs);
				}
			}

			if(!has_consumers)
				snprintf(consumer_info+consumer_info_offs,
					 sizeof(consumer_info)-consumer_info_offs-1,
					 "No consumers found");
		}

		snprintf(buffer, sizeof(buffer),
			 "Active task:\t0x%"PRIx64" <a href=\"task://0x%"PRIx64"\">%s</a>\n"
			 "Task duration:\t%s\n\n"
			 "Active frame: 0x%"PRIx64"\n"
			 "4K page:\t\t0x%"PRIx64"\n"
			 "2M page:\t0x%"PRIx64"\n"
			 "Owner:\t\t%s\n\n"
			 "1st allocation: %s\n"
			 "1st writer:\t %s\n"
			 "1st max writer: %s\n\n"
			 "Reads:\n"
			 "%s\n"
			 "Writes:\n"
			 "%s\n"
			 "Consumer info:\n"
			 "%s",
			 se->active_task->addr,
			 se->active_task->addr,
			 se->active_task->symbol_name,
			 (valid) ? buf_duration : "Invalid active task",
			 se->active_frame->addr,
			 get_base_address(se->active_frame->addr, 1 << 12),
			 get_base_address(se->active_frame->addr, 1 << 21),
			 (valid) ? buf_first_texec_start : "Invalid active task",
			 (valid) ? buf_tcreate : "Invalid active task",
			 (valid) ? buf_first_writer : "Invalid active task",
			 (valid) ? buf_first_max_writer : "Invalid active task",
			 (valid) ? consumption_info : "No consumer information available",
			 (valid) ? production_info : "No consumer information available",
			 (valid) ? consumer_info : "No consumer information available");

		gtk_label_set_markup(GTK_LABEL(g_active_task_label), buffer);
		gtk_trace_set_markers(g_trace_widget, g_trace_markers, num_markers);
	} else {
		gtk_label_set_markup(GTK_LABEL(g_selected_event_label), "");
	}
}

G_MODULE_EXPORT void hscrollbar_value_changed(GtkHScrollbar *item, gdouble value, gpointer data)
{
	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(item));
	double page_size = gtk_adjustment_get_page_size(adj);
	double curr_value = gtk_adjustment_get_value(adj);

	if(react_to_hscrollbar_change)
		gtk_trace_set_bounds(g_trace_widget, curr_value - page_size / 2.0, curr_value + page_size / 2.0);
}

G_MODULE_EXPORT void vscrollbar_value_changed(GtkHScrollbar *item, gdouble value, gpointer data)
{
	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(item));
	double page_size = gtk_adjustment_get_page_size(adj);
	double curr_value = gtk_adjustment_get_value(adj);

	if(react_to_hscrollbar_change)
		gtk_trace_set_cpu_offset(g_trace_widget, curr_value - page_size / 2.0);
}

G_MODULE_EXPORT void task_filter_update(void)
{
	int use_task_length_filter;
	const char* txt;
	int64_t min_task_length;
	int64_t max_task_length;

	filter_clear_tasks(&g_filter);
	task_list_build_filter(GTK_TREE_VIEW(g_task_treeview), &g_filter);

	filter_clear_writes_to_numa_nodes_nodes(&g_filter);
	numa_node_list_build_writes_to_numa_nodes_filter(GTK_TREE_VIEW(g_writes_to_numa_nodes_treeview), &g_filter);

	use_task_length_filter = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_use_task_length_check));

	if(use_task_length_filter) {
		txt = gtk_entry_get_text(GTK_ENTRY(g_task_length_min_entry));
		if(sscanf(txt, "%"PRId64, &min_task_length) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		txt = gtk_entry_get_text(GTK_ENTRY(g_task_length_max_entry));
		if(sscanf(txt, "%"PRId64, &max_task_length) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		filter_set_task_length_filtering_range(&g_filter, min_task_length, max_task_length);
	}

	filter_set_task_length_filtering(&g_filter, use_task_length_filter);

	gtk_trace_set_filter(g_trace_widget, &g_filter);
	update_statistics();
}

G_MODULE_EXPORT void task_filter_button_clicked(GtkMenuItem *item, gpointer data)
{
	task_filter_update();
}

G_MODULE_EXPORT void task_length_entry_activated(GtkEntry *e, gpointer data)
{
	task_filter_update();
}

G_MODULE_EXPORT void comm_filter_update(void)
{
	int use_comm_size_filter;
	const char* txt;
	int64_t min_comm_size;
	int64_t max_comm_size;

	use_comm_size_filter = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_use_comm_size_check));

	if(use_comm_size_filter) {
		txt = gtk_entry_get_text(GTK_ENTRY(g_comm_size_min_entry));
		if(sscanf(txt, "%"PRId64, &min_comm_size) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		txt = gtk_entry_get_text(GTK_ENTRY(g_comm_size_max_entry));
		if(sscanf(txt, "%"PRId64, &max_comm_size) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		filter_set_comm_size_filtering_range(&g_filter, min_comm_size, max_comm_size);
	}

	filter_set_comm_size_filtering(&g_filter, use_comm_size_filter);

	filter_clear_comm_numa_nodes(&g_filter);
	numa_node_list_build_comm_filter(GTK_TREE_VIEW(g_comm_numa_node_treeview), &g_filter);

	gtk_trace_set_filter(g_trace_widget, &g_filter);
}

G_MODULE_EXPORT void comm_size_length_entry_activated(GtkEntry *e, gpointer data)
{
	comm_filter_update();
}

G_MODULE_EXPORT void comm_filter_button_clicked(GtkMenuItem *item, gpointer data)
{
	comm_filter_update();
}

G_MODULE_EXPORT void task_check_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	task_list_check_all(GTK_TREE_VIEW(g_task_treeview));
}

G_MODULE_EXPORT void task_uncheck_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	task_list_uncheck_all(GTK_TREE_VIEW(g_task_treeview));
}

G_MODULE_EXPORT void writes_to_node_uncheck_all_clicked(GtkMenuItem *item, gpointer data)
{
	numa_node_list_uncheck_all(GTK_TREE_VIEW(g_writes_to_numa_nodes_treeview));
}

G_MODULE_EXPORT void writes_to_node_check_all_clicked(GtkMenuItem *item, gpointer data)
{
	numa_node_list_check_all(GTK_TREE_VIEW(g_writes_to_numa_nodes_treeview));
}

G_MODULE_EXPORT void cpu_check_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	cpu_list_check_all(GTK_TREE_VIEW(g_cpu_treeview));
}

G_MODULE_EXPORT void cpu_uncheck_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	cpu_list_uncheck_all(GTK_TREE_VIEW(g_cpu_treeview));
}

G_MODULE_EXPORT void frame_numa_node_check_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	numa_node_list_check_all(GTK_TREE_VIEW(g_frame_numa_node_treeview));
}

G_MODULE_EXPORT void frame_numa_node_uncheck_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	numa_node_list_uncheck_all(GTK_TREE_VIEW(g_frame_numa_node_treeview));
}

G_MODULE_EXPORT void comm_numa_node_check_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	numa_node_list_check_all(GTK_TREE_VIEW(g_comm_numa_node_treeview));
}

G_MODULE_EXPORT void comm_numa_node_uncheck_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	numa_node_list_uncheck_all(GTK_TREE_VIEW(g_comm_numa_node_treeview));
}

G_MODULE_EXPORT void frame_filter_button_clicked(GtkMenuItem *item, gpointer data)
{
	filter_clear_frames(&g_filter);
	frame_list_build_filter(GTK_TREE_VIEW(g_frame_treeview), &g_filter);

	filter_clear_frame_numa_nodes(&g_filter);
	numa_node_list_build_frame_filter(GTK_TREE_VIEW(g_frame_numa_node_treeview), &g_filter);

	gtk_trace_set_filter(g_trace_widget, &g_filter);
	update_statistics();
}

G_MODULE_EXPORT void frame_check_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	frame_list_check_all(GTK_TREE_VIEW(g_frame_treeview));
}

G_MODULE_EXPORT void frame_uncheck_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	frame_list_uncheck_all(GTK_TREE_VIEW(g_frame_treeview));
}

G_MODULE_EXPORT void counter_filter_button_clicked(GtkMenuItem *item, gpointer data)
{
	int use_global_values = 0;
	int use_global_slopes = 0;
	const char* txt;
	int64_t min;
	int64_t max;
	long double min_slope;
	long double max_slope;

	use_global_values = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_use_global_values_check));
	use_global_slopes = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_use_global_slopes_check));

	if(use_global_values) {
		txt = gtk_entry_get_text(GTK_ENTRY(g_global_values_min_entry));
		if(sscanf(txt, "%"PRId64, &min) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		txt = gtk_entry_get_text(GTK_ENTRY(g_global_values_max_entry));
		if(sscanf(txt, "%"PRId64, &max) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		if(min >= max) {
			show_error_message("Maximum value must be greater than the minimum value.");
			return;
		}
	}

	if(use_global_slopes) {
		txt = gtk_entry_get_text(GTK_ENTRY(g_global_slopes_min_entry));
		if(sscanf(txt, "%Lf", &min_slope) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		txt = gtk_entry_get_text(GTK_ENTRY(g_global_slopes_max_entry));
		if(sscanf(txt, "%Lf", &max_slope) != 1) {
			show_error_message("\"%s\" is not a correct integer value.", txt);
			return;
		}

		if(min_slope >= max_slope) {
			show_error_message("Maximum value for slopes must be greater than the minimum value.");
			return;
		}
	}

	g_filter.filter_counter_values = use_global_values;
	g_filter.filter_counter_slopes = use_global_slopes;

	if(use_global_values) {
		g_filter.min = min;
		g_filter.max = max;
	}

	if(use_global_slopes) {
		g_filter.min_slope = min_slope;
		g_filter.max_slope = max_slope;
	}

	filter_clear_counters(&g_filter);
	counter_list_build_filter(GTK_TREE_VIEW(g_counter_treeview), &g_filter);

	gtk_trace_set_filter(g_trace_widget, &g_filter);
	update_statistics();
}

G_MODULE_EXPORT void counter_check_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	counter_list_check_all(GTK_TREE_VIEW(g_counter_treeview));
}

G_MODULE_EXPORT void counter_uncheck_all_button_clicked(GtkMenuItem *item, gpointer data)
{
	counter_list_uncheck_all(GTK_TREE_VIEW(g_counter_treeview));
}

void show_task_code_in_external_editor(struct task* t)
{
	char editor_cmd[FILENAME_MAX];
	char source_line_str[10];

	snprintf(editor_cmd, sizeof(editor_cmd), "%s", g_settings.external_editor_command);
	snprintf(source_line_str, sizeof(source_line_str), "%d", t->source_line);

	strreplace(editor_cmd, "%f", t->source_filename);
	strreplace(editor_cmd, "%l", source_line_str);

	system(editor_cmd);
}

void show_task_code_in_internal_editor(struct task* t)
{
	FILE* fp;
	int file_size;
	char* buffer;
	GtkTextIter text_iter_start;
	GtkTextIter text_iter_end;
	GtkTextMark* mark;
	GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_code_view));

	if(!(fp = fopen(t->source_filename, "r"))) {
		show_error_message("Could not open file %s\n", t->source_filename);
		goto out;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(!(buffer = malloc(file_size+1))) {
		show_error_message("Could not allocate buffer for file %s\n", t->source_filename);
		goto out;
	}

	if(fread(buffer, file_size, 1, fp) != 1) {
		show_error_message("Could read file %s\n", t->source_filename);
		goto out;
	}

	gtk_text_buffer_set_text(text_buffer, buffer, file_size);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(g_main_notebook), 1);

	gtk_text_buffer_get_iter_at_line_offset(text_buffer, &text_iter_start, t->source_line, 0);
	gtk_text_buffer_get_iter_at_line_offset(text_buffer, &text_iter_end, t->source_line+1, 0);
	gtk_text_buffer_select_range(text_buffer, &text_iter_start, &text_iter_end);

	mark = gtk_text_buffer_create_mark(text_buffer, "foo", &text_iter_start, TRUE);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(g_code_view), mark, 0.0, TRUE, 0.2, 0.2);
	gtk_text_buffer_delete_mark(text_buffer, mark);

	free(buffer);

out:
	fclose(fp);
}

void show_task_code(struct task* t)
{
	if(!g_settings.use_external_editor)
		show_task_code_in_internal_editor(t);
	else
		show_task_code_in_external_editor(t);
}

G_MODULE_EXPORT void task_treeview_row_activated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data)
{
	GtkTreeView* task_treeview = tree_view;
	GtkTreeModel* model = gtk_tree_view_get_model(task_treeview);
	GtkTreeIter tree_iter;
	struct task* t;

	gtk_tree_model_get_iter(model, &tree_iter, path);
	gtk_tree_model_get(model, &tree_iter, TASK_LIST_COL_TASK_POINTER, &t, -1);

	if(!t->source_filename)
		return;

	show_task_code(t);
}

int store_visuals_with_dialog(void)
{
	if(!g_visuals_filename) {
		if(!(g_visuals_filename = load_save_file_dialog("Save visuals",
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    "OpenStream visuals",
						    "*.osv",
						      NULL)))
		{
			return 1;
		}
	}

	if(store_visuals(g_visuals_filename, &g_mes)) {
		show_error_message("Could not save visuals to \"%s\".", g_visuals_filename);
		return 1;
	}

	g_visuals_modified = 0;

	return 0;
}

gint check_quit(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if(g_visuals_modified) {
		switch(show_yes_no_cancel_dialog("There are unsaved annotations.\n"
						 "Would you like to save them before quitting?"))
		{
			case DIALOG_RESPONSE_YES:
				if(store_visuals_with_dialog())
					return TRUE;
				break;
			case DIALOG_RESPONSE_CANCEL:
				return TRUE;
			case DIALOG_RESPONSE_NO:
				break;
		}
	}

	gtk_main_quit();
	return FALSE;
}

G_MODULE_EXPORT void menubar_save_visuals(GtkMenuItem *item, gpointer data)
{
	store_visuals_with_dialog();
}

G_MODULE_EXPORT void menubar_save_visuals_as(GtkMenuItem *item, gpointer data)
{
	char* filename;

	if(!(filename = load_save_file_dialog("Save visuals",
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    "OpenStream visuals",
						    "*.osv",
						      NULL)))
	{
		return;
	}

	free(g_visuals_filename);
	g_visuals_filename = filename;

	store_visuals_with_dialog();
}

G_MODULE_EXPORT void define_counter_offset_clicked(GtkMenuItem *item, gpointer data)
{
	struct counter_description* selection = counter_list_get_highlighted_entry(GTK_TREE_VIEW(g_counter_treeview));
	int64_t offset;

	if(!selection) {
		show_error_message("No counter selected");
		return;
	}

	show_counter_offset_dialog(&g_mes, selection, g_trace_widget, &offset);
}

G_MODULE_EXPORT void comm_matrix_pair_under_pointer_changed(GtkMatrix *item, int node_x, int node_y, int64_t absolute, double relative)
{
	char buffer[128];
	char pretty_bytes[32];

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_check_matrix_numonly)))
		snprintf(pretty_bytes, sizeof(pretty_bytes), "%"PRId64, absolute);
	else
		pretty_print_bytes(pretty_bytes, sizeof(pretty_bytes), absolute, "");

	snprintf(buffer, sizeof(buffer), "Node %d to %d:\n%s (%.3f%% max.)\n", node_x, node_y, pretty_bytes, 100.0*relative);
	gtk_label_set_text(GTK_LABEL(g_label_comm_matrix), buffer);
}

void cpu_filter_update(void)
{
	filter_clear_cpus(&g_filter);
	cpu_list_build_filter(GTK_TREE_VIEW(g_cpu_treeview), &g_filter);

	gtk_trace_set_filter(g_trace_widget, &g_filter);
	update_statistics();
}

G_MODULE_EXPORT void cpu_filter_button_clicked(GtkMenuItem *item, gpointer data)
{
	cpu_filter_update();
}
