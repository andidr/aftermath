/**
 * Copyright (C) 2016 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#include <inttypes.h>
#include "omp_task_treeview.h"
#include "omp_task.h"
#include "color.h"
#include "cell_renderer_color_button.h"
#include "util.h"
#include "globals.h"
#include "marshal.h"

void omp_task_treeview_color_changed(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer color, gpointer user_data)
{
	GtkTreeView* omp_task_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(omp_task_treeview);
	GtkTreeIter iter;
	GtkTreeStore* store = GTK_TREE_STORE(model);
	int path_to_object[3] = {-1, -1, -1};

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_store_set(store, &iter, OMP_TASK_TREEVIEW_COL_COLOR, color, -1);

	sscanf(path, "%d:%d:%d",
	       &path_to_object[0],
	       &path_to_object[1],
	       &path_to_object[2]);

	if(path_to_object[1] == -1) {
		struct omp_task * ot;
		gtk_tree_model_get(model, &iter,
				OMP_TASK_TREEVIEW_COL_POINTER, &ot,
				-1);
		ot->color_r = ((GdkColor*)color)->red / 65536.0;
		ot->color_g = ((GdkColor*)color)->green / 65536.0;
		ot->color_b = ((GdkColor*)color)->blue / 65536.0;
	} else if(path_to_object[2] == -1) {
		struct omp_task_instance * oti;
		gtk_tree_model_get(model, &iter,
				OMP_TASK_TREEVIEW_COL_POINTER, &oti,
				-1);
		oti->color_r = ((GdkColor*)color)->red / 65536.0;
		oti->color_g = ((GdkColor*)color)->green / 65536.0;
		oti->color_b = ((GdkColor*)color)->blue / 65536.0;
	} else {
		struct omp_task_part * otp;
		gtk_tree_model_get(model, &iter,
				OMP_TASK_TREEVIEW_COL_POINTER, &otp,
				-1);
		otp->color_r = ((GdkColor*)color)->red / 65536.0;
		otp->color_g = ((GdkColor*)color)->green / 65536.0;
		otp->color_b = ((GdkColor*)color)->blue / 65536.0;
	}

	gtk_widget_queue_draw(g_trace_widget);
}

void omp_task_treeview_toggle_children(GtkTreeModel* model, GtkTreeStore* store, GtkTreeIter* iter, gboolean current_state, int level)
{
		do {
			gtk_tree_store_set(store, &iter[level], OMP_TASK_TREEVIEW_COL_FILTER, !current_state, -1);
			if(level < 2) {
				if(gtk_tree_model_iter_children(model, &iter[level + 1], &iter[level]) == FALSE)
					continue;
				omp_task_treeview_toggle_children(model, store, iter, current_state, level + 1);
			}
		} while(gtk_tree_model_iter_next(model, &iter[level]));
}

void omp_task_treeview_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* omp_task_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(omp_task_treeview);
	GtkTreeIter iter[3];

	GtkTreeStore* store = GTK_TREE_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter[0], path);
	gtk_tree_model_get(model, &iter[0], OMP_TASK_TREEVIEW_COL_FILTER, &current_state, -1);
	gtk_tree_store_set(store, &iter[0], OMP_TASK_TREEVIEW_COL_FILTER, !current_state, -1);

	if(gtk_tree_model_iter_children(model, &iter[1], &iter[0]) == FALSE)
		return;

	omp_task_treeview_toggle_children(model, store, iter, current_state, 1);
}

GtkWidget* omp_task_treeview_init(GtkTreeView* omp_task_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;

	GtkTreeStore* store = gtk_tree_store_new(OMP_TASK_TREEVIEW_COL_NUM,
						 G_TYPE_BOOLEAN,
						 GDK_TYPE_COLOR,
						 G_TYPE_STRING,
						 G_TYPE_STRING,
						 G_TYPE_STRING,
						 G_TYPE_STRING,
						 G_TYPE_STRING,
						 G_TYPE_STRING,
						 G_TYPE_STRING,
						 G_TYPE_POINTER,
						 G_TYPE_INT);

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", OMP_TASK_TREEVIEW_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(omp_task_treeview_toggle), omp_task_treeview);

	renderer = custom_cell_renderer_color_button_new();
	column = gtk_tree_view_column_new_with_attributes("Color", renderer, "color", OMP_TASK_TREEVIEW_COL_COLOR, NULL);
	gtk_tree_view_append_column(omp_task_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "color-changed", G_CALLBACK(omp_task_treeview_color_changed), GTK_TREE_VIEW(omp_task_treeview));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", OMP_TASK_TREEVIEW_COL_ADDR, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Symbol", renderer, "text", OMP_TASK_TREEVIEW_COL_SYMBOL, NULL);
	gtk_tree_view_column_set_fixed_width(column, 100);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Start", renderer, "text", OMP_TASK_TREEVIEW_COL_START, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("End", renderer, "text", OMP_TASK_TREEVIEW_COL_END, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("CPU", renderer, "text", OMP_TASK_TREEVIEW_COL_CPU, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Source file", renderer, "text", OMP_TASK_TREEVIEW_COL_SOURCE_FILE, NULL);
	gtk_tree_view_column_set_fixed_width(column, 100);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Line", renderer, "text", OMP_TASK_TREEVIEW_COL_SOURCE_LINE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_task_treeview, column);

	gtk_tree_view_set_model(omp_task_treeview, GTK_TREE_MODEL(store));
	g_object_unref(store);

	return NULL;
}

void omp_task_add_task_parts(GtkTreeStore* store, struct omp_task_instance* oti, GtkTreeIter* iter_task_instance, GdkColor* color)
{
	GtkTreeIter iter_part;
	struct list_head* l_iter_part;
	struct omp_task_part* otp;
	char tmp[20];
	char buff_start[20];
	char buff_end[20];
	char cpu[4];

	list_for_each(l_iter_part, &oti->task_parts) {
		otp = list_entry(l_iter_part, struct omp_task_part, list);

		pretty_print_cycles(tmp, sizeof(buff_start), otp->start);
		snprintf(buff_start, sizeof(buff_start), "%s%s", tmp, "cycles");
		pretty_print_cycles(tmp, sizeof(buff_end), otp->end);
		snprintf(buff_end, sizeof(buff_end), "%s%s", tmp, "cycles");
		snprintf(cpu, sizeof(cpu), "%d", otp->cpu);

		gtk_tree_store_append(store, &iter_part, iter_task_instance);
		gtk_tree_store_set(store, &iter_part,
				OMP_TASK_TREEVIEW_COL_FILTER, TRUE,
				OMP_TASK_TREEVIEW_COL_COLOR, color,
				OMP_TASK_TREEVIEW_COL_ADDR, "",
				OMP_TASK_TREEVIEW_COL_START, buff_start,
				OMP_TASK_TREEVIEW_COL_END, buff_end,
				OMP_TASK_TREEVIEW_COL_CPU, cpu,
				OMP_TASK_TREEVIEW_COL_POINTER, otp,
				OMP_TASK_TREEVIEW_COL_LEVEL, 2,
				-1);
	}
}

void omp_task_add_instances(GtkTreeStore* store, struct omp_task* ot, GtkTreeIter* iter_task)
{
	GtkTreeIter iter_taski;
	GdkColor color;
	struct list_head* l_iter_taski;
	struct omp_task_instance* oti;

	list_for_each(l_iter_taski, &(ot->task_instances)) {
		oti = list_entry(l_iter_taski, struct omp_task_instance, list);

		color.red = oti->color_r * 65535.0;
		color.green = oti->color_g * 65535.0;
		color.blue = oti->color_b * 65535.0;
		gtk_tree_store_append(store, &iter_taski, iter_task);
		gtk_tree_store_set(store, &iter_taski,
				OMP_TASK_TREEVIEW_COL_FILTER, TRUE,
				OMP_TASK_TREEVIEW_COL_COLOR, &color,
				OMP_TASK_TREEVIEW_COL_ADDR, "",
				OMP_TASK_TREEVIEW_COL_START, "",
				OMP_TASK_TREEVIEW_COL_END, "",
				OMP_TASK_TREEVIEW_COL_CPU, "",
				OMP_TASK_TREEVIEW_COL_POINTER, oti,
				OMP_TASK_TREEVIEW_COL_LEVEL, 1,
				-1);

		omp_task_add_task_parts(store, oti, &iter_taski, &color);
	}
}

void omp_task_add_tasks(GtkTreeStore* store, struct omp_task* omp_tasks, int num_omp_tasks)
{
	GtkTreeIter iter_task;
	char buff_addr[20];
	char line[20];
	GdkColor color;

	for(int i = 0; i < num_omp_tasks; i++) {
		snprintf(buff_addr, sizeof(buff_addr), "0x%"PRIx64, omp_tasks[i].addr);
		snprintf(line, sizeof(line), "%u", omp_tasks[i].source_line);
		color.red = omp_tasks[i].color_r * 65535.0;
		color.green = omp_tasks[i].color_g * 65535.0;
		color.blue = omp_tasks[i].color_b * 65535.0;
		gtk_tree_store_append(store, &iter_task, NULL);
		gtk_tree_store_set(store, &iter_task,
				   OMP_TASK_TREEVIEW_COL_FILTER, TRUE,
				   OMP_TASK_TREEVIEW_COL_COLOR, &color,
				   OMP_TASK_TREEVIEW_COL_ADDR, buff_addr,
				   OMP_TASK_TREEVIEW_COL_SYMBOL, (omp_tasks[i].symbol_name == NULL)?
							     "debug symbol not found" :
							     omp_tasks[i].symbol_name,
				   OMP_TASK_TREEVIEW_COL_START, "",
				   OMP_TASK_TREEVIEW_COL_END, "",
				   OMP_TASK_TREEVIEW_COL_CPU, "",
				   OMP_TASK_TREEVIEW_COL_SOURCE_FILE, (omp_tasks[i].source_filename == NULL)?
							          "debug symbol not found" :
							          omp_tasks[i].source_filename,
				   OMP_TASK_TREEVIEW_COL_SOURCE_LINE, (omp_tasks[i].source_line == -1)?
								  "debug symbol not found" :
								  line,
				   OMP_TASK_TREEVIEW_COL_POINTER, &omp_tasks[i],
				   OMP_TASK_TREEVIEW_COL_LEVEL, 0,
				   -1);
		omp_task_add_instances(store, &omp_tasks[i], &iter_task);
	}
}

void omp_task_treeview_fill(GtkTreeView* omp_task_treeview, struct omp_task* omp_tasks, int num_omp_tasks)
{
	GtkTreeModel* model = gtk_tree_view_get_model(omp_task_treeview);
	GtkTreeStore* store = GTK_TREE_STORE(model);

	omp_task_add_tasks(store, omp_tasks, num_omp_tasks);
}

int omp_task_treeview_add_to_filter(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer filter)
{
	gboolean current_state;
	int level;
	struct omp_task_part * otp;

	gtk_tree_model_get(model, iter,
			   OMP_TASK_TREEVIEW_COL_FILTER, &current_state,
			   OMP_TASK_TREEVIEW_COL_LEVEL, &level,
			   OMP_TASK_TREEVIEW_COL_POINTER, &otp,
			   -1);

	if(level == 2 && current_state)
		filter_add_otp((struct filter*)filter, otp);

	return FALSE;
}

void omp_task_treeview_build_filter(GtkTreeView* omp_task_treeview, struct filter* filter)
{
	GtkTreeModel* model = gtk_tree_view_get_model(omp_task_treeview);

	gtk_tree_model_foreach(model, omp_task_treeview_add_to_filter, filter);

	filter_sort_otps(filter);
	filter_set_otp_filtering(filter, 1);
}
