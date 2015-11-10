/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr> + 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#include "state_list.h"
#include "globals.h"
#include "cell_renderer_color_button.h"
#include <inttypes.h>

void state_list_color_changed(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer color, gpointer user_data)
{
	GtkTreeView* state_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(state_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);

	g_mes.states[atoi(path)].color_r = ((GdkColor*)color)->red / 65536.0;
	g_mes.states[atoi(path)].color_g = ((GdkColor*)color)->green / 65536.0;
	g_mes.states[atoi(path)].color_b = ((GdkColor*)color)->blue / 65536.0;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_list_store_set(store, &iter, STATE_LIST_COL_COLOR, color, -1);

	gtk_trace_paint(g_trace_widget);

}

void state_list_init(GtkTreeView* state_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = custom_cell_renderer_color_button_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "color", STATE_LIST_COL_COLOR, NULL);
	gtk_tree_view_append_column(state_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "color-changed", G_CALLBACK(state_list_color_changed), state_treeview);


	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", STATE_LIST_COL_NAME, NULL);
	gtk_tree_view_append_column(state_treeview, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Per", renderer, "text", STATE_LIST_COL_PER, NULL);
	gtk_tree_view_append_column(state_treeview, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Par", renderer, "text", STATE_LIST_COL_PAR, NULL);
	gtk_tree_view_append_column(state_treeview, column);

	store = gtk_list_store_new(STATE_LIST_COL_NUM, GDK_TYPE_COLOR, G_TYPE_STRING, 
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, -1);

	gtk_tree_view_set_model(state_treeview, GTK_TREE_MODEL(store));

	g_object_unref(store);
}

void __state_list_append(GtkListStore* store, struct state_description* state, int init)
{
	GtkTreeIter iter;
	char buff_per[32];
	char buff_par[32];

	gtk_list_store_append(store, &iter);

	if(init) {
		snprintf(buff_per, sizeof(buff_per), " ");
		snprintf(buff_par, sizeof(buff_par), " ");
	} else {
		snprintf(buff_per, sizeof(buff_per), "%.2f%%", state->per);
		snprintf(buff_par, sizeof(buff_par), "%.2f", state->par);
	}

	GdkColor color = {
		.red = state->color_r * 65535,
		.green = state->color_g * 65535,
		.blue = state->color_b * 65535
	};

	gtk_list_store_set(store, &iter,
			   STATE_LIST_COL_COLOR, &color,
			   STATE_LIST_COL_NAME, state->name,
			   STATE_LIST_COL_PER, buff_per,
			   STATE_LIST_COL_PAR, buff_par,
			   STATE_LIST_COL_STATEDESC_POINTER, state,
			   -1);
}

void state_list_append(GtkTreeView* state_treeview, struct state_description* state, int init)
{
	GtkTreeModel* model = gtk_tree_view_get_model(state_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	__state_list_append(store, state, init);
}

void state_list_update_colors(GtkTreeView* task_treeview)
{
	GtkTreeModel* model = gtk_tree_view_get_model(task_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	GdkColor color;
	struct state_description* sd;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter, STATE_LIST_COL_STATEDESC_POINTER, &sd, -1);

		color.red = sd->color_r * 65535.0;
		color.green = sd->color_g * 65535.0;
		color.blue = sd->color_b * 65535.0;

		gtk_list_store_set(store, &iter, STATE_LIST_COL_COLOR, &color, -1);
	} while(gtk_tree_model_iter_next(model, &iter));
}

void state_list_fill(GtkTreeView* state_treeview, struct state_description* states, int num_states)
{
	GtkTreeModel* model = gtk_tree_view_get_model(state_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	for(int i = 0; i < num_states; i++)
		__state_list_append(store, &states[i], 0);
}

void state_list_fill_name(GtkTreeView* state_treeview, struct state_description* states, int num_states)
{
	GtkTreeModel* model = gtk_tree_view_get_model(state_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	for(int i = 0; i < num_states; i++)
		__state_list_append(store, &states[i], 1);
}

void state_list_clear(GtkTreeView* state_treeview)
{
	GtkTreeModel* model = gtk_tree_view_get_model(state_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	gtk_list_store_clear(store);
}
