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

#include "counter_list.h"
#include "cell_renderer_color_button.h"
#include <inttypes.h>

void counter_list_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* counter_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, COUNTER_LIST_COL_FILTER, &current_state, -1);
	gtk_list_store_set(store, &iter, COUNTER_LIST_COL_FILTER, !current_state, -1);
}

void counter_list_toggle_slope(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* counter_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, COUNTER_LIST_COL_MODE, &current_state, -1);
	gtk_list_store_set(store, &iter, COUNTER_LIST_COL_MODE, !current_state, -1);
}

void counter_list_color_changed(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer color, gpointer user_data)
{
	GtkTreeView* counter_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_list_store_set(store, &iter, COUNTER_LIST_COL_COLOR, color, -1);
}

void counter_list_init(GtkTreeView* counter_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", COUNTER_LIST_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(counter_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(counter_list_toggle), counter_treeview);

	renderer = custom_cell_renderer_color_button_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "color", COUNTER_LIST_COL_COLOR, NULL);
	gtk_tree_view_append_column(counter_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "color-changed", G_CALLBACK(counter_list_color_changed), counter_treeview);


	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COUNTER_LIST_COL_NAME, NULL);
	gtk_tree_view_append_column(counter_treeview, column);

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("Slope mode", renderer, "active", COUNTER_LIST_COL_MODE, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(counter_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(counter_list_toggle_slope), counter_treeview);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Min", renderer, "text", COUNTER_LIST_COL_MIN, NULL);
	gtk_tree_view_append_column(counter_treeview, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Max", renderer, "text", COUNTER_LIST_COL_MAX, NULL);
	gtk_tree_view_append_column(counter_treeview, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Min slope", renderer, "text", COUNTER_LIST_COL_MIN_SLOPE, NULL);
	gtk_tree_view_append_column(counter_treeview, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Max slope", renderer, "text", COUNTER_LIST_COL_MAX_SLOPE, NULL);
	gtk_tree_view_append_column(counter_treeview, column);

	store = gtk_list_store_new(COUNTER_LIST_COL_NUM, G_TYPE_BOOLEAN, GDK_TYPE_COLOR, G_TYPE_STRING, G_TYPE_BOOLEAN,
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	gtk_tree_view_set_model(counter_treeview, GTK_TREE_MODEL(store));

	g_object_unref(store);
}

void __counter_list_append(GtkListStore* store, struct counter_description* counter, gboolean active)
{
	GtkTreeIter iter;
	char buff_min[32];
	char buff_max[32];
	char buff_min_slope[32];
	char buff_max_slope[32];

	gtk_list_store_append(store, &iter);

	snprintf(buff_min, sizeof(buff_min), "%"PRId64, counter->min);
	snprintf(buff_max, sizeof(buff_max), "%"PRId64, counter->max);
	snprintf(buff_min_slope, sizeof(buff_min_slope), "%Lf", counter->min_slope);
	snprintf(buff_max_slope, sizeof(buff_max_slope), "%Lf", counter->max_slope);

	GdkColor color = { .red = counter->color_r*65535,
			   .green = counter->color_g*65535,
			   .blue = counter->color_b*65535 };

	gtk_list_store_set(store, &iter,
			   COUNTER_LIST_COL_FILTER, active,
			   COUNTER_LIST_COL_COLOR, &color,
			   COUNTER_LIST_COL_NAME, counter->name,
			   COUNTER_LIST_COL_MODE, FALSE,
			   COUNTER_LIST_COL_MIN, buff_min,
			   COUNTER_LIST_COL_MAX, buff_max,
			   COUNTER_LIST_COL_MIN_SLOPE, buff_min_slope,
			   COUNTER_LIST_COL_MAX_SLOPE, buff_max_slope,
			   COUNTER_LIST_COL_COUNTER_POINTER, counter,
			   -1);
}

void counter_list_append(GtkTreeView* counter_treeview, struct counter_description* counter, gboolean active)
{
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	__counter_list_append(store, counter, active);
}

void counter_list_fill(GtkTreeView* counter_treeview, struct counter_description* counters, int num_counters)
{
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	for(int i = 0; i < num_counters; i++)
		__counter_list_append(store, &counters[i], TRUE);
}

void counter_list_clear(GtkTreeView* counter_treeview)
{
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	gtk_list_store_clear(store);
}

void counter_list_build_filter(GtkTreeView* counter_treeview, struct filter* filter)
{
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkTreeIter iter;
	gboolean current_state;
	gboolean slope_mode;
	struct counter_description* cd;
	GdkColor* color;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter,
				   COUNTER_LIST_COL_FILTER, &current_state,
				   COUNTER_LIST_COL_MODE, &slope_mode,
				   COUNTER_LIST_COL_COLOR, &color,
				   COUNTER_LIST_COL_COUNTER_POINTER, &cd, -1);

		if(current_state) {
			cd->slope_mode = slope_mode;
			cd->color_r = color->red / 65535.0;
			cd->color_g = color->green / 65535.0;
			cd->color_b = color->blue / 65535.0;
			filter_add_counter(filter, cd);
		}

	} while(gtk_tree_model_iter_next(model, &iter));
}

struct counter_description* counter_list_get_highlighted_entry(GtkTreeView* counter_treeview)
{
	GtkTreeSelection* selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	struct counter_description* cd = NULL;

	selection = gtk_tree_view_get_selection(counter_treeview);

	if(gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_tree_model_get(model, &iter, COUNTER_LIST_COL_COUNTER_POINTER, &cd, -1);

	return cd;
}

void counter_list_set_status_all(GtkTreeView* counter_treeview, gboolean status)
{
	GtkTreeModel* model = gtk_tree_view_get_model(counter_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_list_store_set(store, &iter, COUNTER_LIST_COL_FILTER, status, -1);
	} while(gtk_tree_model_iter_next(model, &iter));
}

void counter_list_check_all(GtkTreeView* counter_treeview)
{
	counter_list_set_status_all(counter_treeview, TRUE);
}

void counter_list_uncheck_all(GtkTreeView* counter_treeview)
{
	counter_list_set_status_all(counter_treeview, FALSE);
}
