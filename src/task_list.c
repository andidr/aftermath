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

#include "task_list.h"
#include <inttypes.h>

enum task_list_columns {
	TASK_LIST_COL_FILTER = 0,
	TASK_LIST_COL_ADDR,
	TASK_LIST_COL_SYMBOL,
	TASK_LIST_COL_SOURCE_FILE,
	TASK_LIST_COL_SOURCE_LINE,
	TASK_LIST_COL_TASK_POINTER,
	TASK_LIST_COL_NUM
};

void task_list_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* task_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(task_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, TASK_LIST_COL_FILTER, &current_state, -1);
	gtk_list_store_set(store, &iter, TASK_LIST_COL_FILTER, !current_state, -1);
}

void task_list_init(GtkTreeView* task_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", TASK_LIST_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(task_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(task_list_toggle), task_treeview);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", TASK_LIST_COL_ADDR, NULL);
	gtk_tree_view_append_column(task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Symbol", renderer, "text", TASK_LIST_COL_SYMBOL, NULL);
	gtk_tree_view_append_column(task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Source file", renderer, "text", TASK_LIST_COL_SOURCE_FILE, NULL);
	gtk_tree_view_append_column(task_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Line", renderer, "text", TASK_LIST_COL_SOURCE_LINE, NULL);
	gtk_tree_view_append_column(task_treeview, column);

	store = gtk_list_store_new(TASK_LIST_COL_NUM, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	gtk_tree_view_set_model(task_treeview, GTK_TREE_MODEL(store));

	g_object_unref(store);
}

void task_list_fill(GtkTreeView* task_treeview, struct task* tasks, int num_tasks)
{
	GtkTreeModel* model = gtk_tree_view_get_model(task_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	char buff_addr[19];
	char buff_line[10];
	char* source_file;
	char* symbol;

	for(int i = 0; i < num_tasks; i++) {
		snprintf(buff_addr, sizeof(buff_addr), "0x%"PRIx64, tasks[i].work_fn);
		symbol = (tasks[i].symbol_name) ? tasks[i].symbol_name : "(no symbol found)";

		if(tasks[i].source_filename) {
			snprintf(buff_line, sizeof(buff_line), "%d", tasks[i].source_line);
			source_file = tasks[i].source_filename;
		} else {
			buff_line[0] = '\0';
			source_file = "";
		}

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   TASK_LIST_COL_FILTER, TRUE,
				   TASK_LIST_COL_ADDR, buff_addr,
				   TASK_LIST_COL_SYMBOL, symbol,
				   TASK_LIST_COL_SOURCE_FILE, source_file,
				   TASK_LIST_COL_SOURCE_LINE, buff_line,
				   TASK_LIST_COL_TASK_POINTER, &tasks[i],
				   -1);
	}
}

void task_list_build_filter(GtkTreeView* task_treeview, struct filter* filter)
{
	GtkTreeModel* model = gtk_tree_view_get_model(task_treeview);
	GtkTreeIter iter;
	gboolean current_state;
	struct task* t;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter,
				   TASK_LIST_COL_FILTER, &current_state,
				   TASK_LIST_COL_TASK_POINTER, &t, -1);

		if(current_state)
			filter_add_task(filter, t);

	} while(gtk_tree_model_iter_next(model, &iter));

	filter_sort_tasks(filter);
}

void task_list_set_status_all(GtkTreeView* task_treeview, gboolean status)
{
	GtkTreeModel* model = gtk_tree_view_get_model(task_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_list_store_set(store, &iter, TASK_LIST_COL_FILTER, status, -1);
	} while(gtk_tree_model_iter_next(model, &iter));
}

void task_list_check_all(GtkTreeView* task_treeview)
{
	task_list_set_status_all(task_treeview, TRUE);
}

void task_list_uncheck_all(GtkTreeView* task_treeview)
{
	task_list_set_status_all(task_treeview, FALSE);
}
