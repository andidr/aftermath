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

#ifndef SIGNALS_H
#define SIGNALS_H

#include <gtk/gtk.h>
#include "trace_widget.h"
#include "histogram_widget.h"
#include "multi_histogram_widget.h"
#include "matrix_widget.h"
#include "events.h"

void reset_zoom(void);
void trace_bounds_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data);
void trace_ybounds_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data);
void trace_state_event_under_pointer_changed(GtkTrace* item, gpointer pstate_event, int cpu, int worker, gpointer data);
void trace_state_event_selection_changed(GtkTrace* item, gpointer pstate_event, int cpu, int worker, gpointer data);
void trace_omp_chunk_set_part_selection_changed(GtkTrace* item, gpointer pstate_event, int cpu, int worker, gpointer data);
void omp_update_highlighted_part(GtkTreeView* item, gpointer pomp_chunk_part, gpointer data);
void trace_range_selection_changed(GtkTrace *item, gdouble left, gdouble right, gpointer data);
void histogram_range_selection_changed(GtkHistogram *item, gdouble left, gdouble right, gpointer data);
void multi_histogram_range_selection_changed(GtkMultiHistogram *item, gdouble left, gdouble right, gpointer data);
void task_link_activated(uint64_t work_fn);
void time_link_activated(uint64_t time);
gint link_activated(GtkLabel *label, gchar *uri, gpointer user_data);
void task_treeview_row_activated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data);
void trace_create_annotation(GtkTrace *item, int cpu, gdouble time);
void trace_edit_annotation(GtkTrace *item, struct annotation* a);
void show_task_code(struct task* t);
gint check_quit(GtkWidget *widget, GdkEvent *event, gpointer data);
void comm_matrix_pair_under_pointer_changed(GtkMatrix *item, int node_x, int node_y, int64_t absolute, double relative);
void comm_summary_matrix_pair_under_pointer_changed(GtkMatrix *item, int node_x, int node_y, int64_t absolute, double relative);

#endif
