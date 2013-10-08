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

#include "numa_node_list.h"

void numa_node_list_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* numa_node_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(numa_node_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, NUMA_NODE_LIST_COL_FILTER, &current_state, -1);
	gtk_list_store_set(store, &iter, NUMA_NODE_LIST_COL_FILTER, !current_state, -1);
}

void numa_node_list_init(GtkTreeView* numa_node_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", NUMA_NODE_LIST_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(numa_node_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(numa_node_list_toggle), numa_node_treeview);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Node", renderer, "text", NUMA_NODE_LIST_COL_NUMA_NODE, NULL);
	gtk_tree_view_append_column(numa_node_treeview, column);

	store = gtk_list_store_new(NUMA_NODE_LIST_COL_NUM, G_TYPE_BOOLEAN, G_TYPE_STRING);

	gtk_tree_view_set_model(numa_node_treeview, GTK_TREE_MODEL(store));

	g_object_unref(store);
}

void numa_node_list_fill(GtkTreeView* numa_node_treeview, int max_numa_node_id)
{
	GtkTreeModel* model = gtk_tree_view_get_model(numa_node_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	char buff_node[32];

	for(int i = 0; i <= max_numa_node_id; i++) {
		snprintf(buff_node, sizeof(buff_node), "Node %d", i);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   NUMA_NODE_LIST_COL_FILTER, TRUE,
				   NUMA_NODE_LIST_COL_NUMA_NODE, buff_node,
				   -1);
	}
}

enum filter_mode {
	FILTER_MODE_FRAME,
	FILTER_MODE_COMM,
	FILTER_MODE_WRITES_TO_NODE
};

void __numa_node_list_build_filter(GtkTreeView* numa_node_treeview, struct filter* filter, enum filter_mode mode)
{
	GtkTreeModel* model = gtk_tree_view_get_model(numa_node_treeview);
	GtkTreeIter iter;
	gboolean current_state;
	int node_id = 0;
	gboolean has_unchecked = FALSE;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter,
				   NUMA_NODE_LIST_COL_FILTER, &current_state,
				   -1);

		if(current_state) {
			if(mode == FILTER_MODE_FRAME)
				filter_add_frame_numa_node(filter, node_id);
			else if(mode == FILTER_MODE_COMM)
				filter_add_comm_numa_node(filter, node_id);
			else if(mode == FILTER_MODE_WRITES_TO_NODE)
				filter_add_writes_to_numa_nodes_node(filter, node_id);
		} else {
			has_unchecked = TRUE;
		}

		node_id++;
	} while(gtk_tree_model_iter_next(model, &iter));

	if(!has_unchecked) {
		if(mode == FILTER_MODE_FRAME)
			filter_set_frame_numa_node_filtering(filter, 0);
		else if(mode == FILTER_MODE_COMM)
			filter_set_comm_numa_node_filtering(filter, 0);
		else if(mode == FILTER_MODE_WRITES_TO_NODE)
			filter_set_writes_to_numa_nodes_filtering(filter, 0);
	}
}

void numa_node_list_build_frame_filter(GtkTreeView* numa_node_treeview, struct filter* filter)
{
	__numa_node_list_build_filter(numa_node_treeview, filter, FILTER_MODE_FRAME);
}

void numa_node_list_build_comm_filter(GtkTreeView* numa_node_treeview, struct filter* filter)
{
	__numa_node_list_build_filter(numa_node_treeview, filter, FILTER_MODE_COMM);
}

void numa_node_list_build_writes_to_numa_nodes_filter(GtkTreeView* numa_node_treeview, struct filter* filter)
{
	__numa_node_list_build_filter(numa_node_treeview, filter, FILTER_MODE_WRITES_TO_NODE);
}

void numa_node_list_set_status_all(GtkTreeView* numa_node_treeview, gboolean status)
{
	GtkTreeModel* model = gtk_tree_view_get_model(numa_node_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_list_store_set(store, &iter, NUMA_NODE_LIST_COL_FILTER, status, -1);
	} while(gtk_tree_model_iter_next(model, &iter));
}

void numa_node_list_check_all(GtkTreeView* numa_node_treeview)
{
	numa_node_list_set_status_all(numa_node_treeview, TRUE);
}

void numa_node_list_uncheck_all(GtkTreeView* numa_node_treeview)
{
	numa_node_list_set_status_all(numa_node_treeview, FALSE);
}
