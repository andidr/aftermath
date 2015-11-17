/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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

#include "color_scheme_set_list.h"
#include "color.h"
#include <inttypes.h>
#include "cell_renderer_color_button.h"
#include "dialogs.h"

void color_scheme_set_list_name_edited(GtkCellRendererText *cell,
				       gchar* path,
				       gchar* new_name,
				       gpointer user_data)
{
	GtkTreeView* css_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(css_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	struct color_scheme* cs;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, COLOR_SCHEME_SET_LIST_COL_CS_POINTER, &cs, -1);

	if(strcmp(cs->name, new_name)) {
		if(color_scheme_set_name(cs, new_name))
			show_error_message("Could not set name for color scheme");
		else
			gtk_list_store_set(store, &iter, COLOR_SCHEME_SET_LIST_COL_NAME, cs->name, -1);
	}
}

void color_scheme_set_list_init(GtkTreeView* css_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COLOR_SCHEME_SET_LIST_COL_NAME, NULL);
	gtk_tree_view_append_column(css_treeview, column);

	store = gtk_list_store_new(COLOR_SCHEME_SET_LIST_COL_NUM, G_TYPE_STRING, G_TYPE_POINTER);

	gtk_tree_view_set_model(css_treeview, GTK_TREE_MODEL(store));

	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(color_scheme_set_list_name_edited), css_treeview);

	g_object_unref(store);
}

void color_scheme_set_list_fill(GtkTreeView* css_treeview, struct color_scheme_set* css)
{
	GtkTreeModel* model = gtk_tree_view_get_model(css_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;

	for(int i = 0; i < css->num_schemes; i++) {
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   COLOR_SCHEME_SET_LIST_COL_NAME, css->schemes[i]->name,
				   COLOR_SCHEME_SET_LIST_COL_CS_POINTER, css->schemes[i],
				   -1);
	}
}

void color_scheme_set_list_clear(GtkTreeView* css_treeview)
{
	GtkTreeModel* model = gtk_tree_view_get_model(css_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	gtk_list_store_clear(store);
}

void color_scheme_set_list_reset(GtkTreeView* css_treeview, struct color_scheme_set* css)
{
	color_scheme_set_list_clear(css_treeview);
	color_scheme_set_list_fill(css_treeview, css);
}

struct color_scheme* color_scheme_set_list_get_selected(GtkTreeView* css_treeview)
{
	GtkTreeSelection *selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	struct color_scheme* cs;

	selection = gtk_tree_view_get_selection(css_treeview);

	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, COLOR_SCHEME_SET_LIST_COL_CS_POINTER, &cs, -1);
		return cs;
	}

	return NULL;
}

struct color_scheme* color_scheme_set_list_get(GtkTreeView* css_treeview, GtkTreePath* path)
{
	GtkTreeModel* model = gtk_tree_view_get_model(css_treeview);
	GtkTreeIter iter;
	struct color_scheme* cs;

	if(!gtk_tree_model_get_iter(model, &iter, path))
		return NULL;

	gtk_tree_model_get(model, &iter, COLOR_SCHEME_SET_LIST_COL_CS_POINTER, &cs, -1);

	return cs;
}

void color_scheme_set_list_remove(GtkTreeView* css_treeview, struct color_scheme* cs)
{
	GtkTreeModel* model = gtk_tree_view_get_model(css_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	struct color_scheme* csiter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter, COLOR_SCHEME_SET_LIST_COL_CS_POINTER, &csiter, -1);

		if(csiter == cs) {
			gtk_list_store_remove(store, &iter);
			return;
		}
	} while(gtk_tree_model_iter_next(model, &iter));
}
