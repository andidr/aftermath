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

#include "frame_list.h"
#include <inttypes.h>

void frame_list_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* frame_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(frame_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, FRAME_LIST_COL_FILTER, &current_state, -1);
	gtk_list_store_set(store, &iter, FRAME_LIST_COL_FILTER, !current_state, -1);
}

void frame_list_init(GtkTreeView* frame_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", FRAME_LIST_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(frame_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(frame_list_toggle), frame_treeview);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", FRAME_LIST_COL_ADDR, NULL);
	gtk_tree_view_append_column(frame_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Steals", renderer, "text", FRAME_LIST_COL_NUM_STEALS, NULL);
	gtk_tree_view_append_column(frame_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Pushes", renderer, "text", FRAME_LIST_COL_NUM_PUSHES, NULL);
	gtk_tree_view_append_column(frame_treeview, column);

	store = gtk_list_store_new(FRAME_LIST_COL_NUM, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	gtk_tree_view_set_model(frame_treeview, GTK_TREE_MODEL(store));

	g_object_unref(store);
}

void frame_list_fill(GtkTreeView* frame_treeview, struct frame* frames, int num_frames)
{
	GtkTreeModel* model = gtk_tree_view_get_model(frame_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	char buff_addr[19];
	char buff_num_steals[10];
	char buff_num_pushes[10];

	for(int i = 0; i < num_frames; i++) {
		snprintf(buff_addr, sizeof(buff_addr), "0x%"PRIx64, frames[i].addr);
		snprintf(buff_num_steals, sizeof(buff_num_steals), "%d", frames[i].num_steals);
		snprintf(buff_num_pushes, sizeof(buff_num_pushes), "%d", frames[i].num_pushes);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   FRAME_LIST_COL_FILTER, TRUE,
				   FRAME_LIST_COL_ADDR, buff_addr,
				   FRAME_LIST_COL_NUM_STEALS, buff_num_steals,
				   FRAME_LIST_COL_NUM_PUSHES, buff_num_pushes,
				   FRAME_LIST_COL_FRAME_POINTER, &frames[i],
				   -1);
	}
}

void frame_list_build_filter(GtkTreeView* frame_treeview, struct filter* filter)
{
	GtkTreeModel* model = gtk_tree_view_get_model(frame_treeview);
	GtkTreeIter iter;
	gboolean current_state;
	struct frame* f;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter,
				   FRAME_LIST_COL_FILTER, &current_state,
				   FRAME_LIST_COL_FRAME_POINTER, &f, -1);

		if(current_state)
			filter_add_frame(filter, f);
	} while(gtk_tree_model_iter_next(model, &iter));

	filter_sort_frames(filter);
}

void frame_list_set_status_all(GtkTreeView* frame_treeview, gboolean status)
{
	GtkTreeModel* model = gtk_tree_view_get_model(frame_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_list_store_set(store, &iter, FRAME_LIST_COL_FILTER, status, -1);
	} while(gtk_tree_model_iter_next(model, &iter));
}

void frame_list_check_all(GtkTreeView* frame_treeview)
{
	frame_list_set_status_all(frame_treeview, TRUE);
}

void frame_list_uncheck_all(GtkTreeView* frame_treeview)
{
	frame_list_set_status_all(frame_treeview, FALSE);
}
