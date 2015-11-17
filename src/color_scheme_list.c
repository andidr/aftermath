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

#include "color_scheme_list.h"
#include "color.h"
#include <inttypes.h>
#include "cell_renderer_color_button.h"
#include "dialogs.h"

void color_scheme_list_type_changed(GtkCellRendererText *cell,
				   gchar* path,
				   gchar* snew_type,
				   gpointer user_data)
{
	GtkTreeView* cs_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	enum color_scheme_rule_type new_type;
	struct color_scheme_rule* csr;

	if(color_scheme_rule_type_from_name(snew_type, &new_type)) {
		show_error_message("Could not find type");
	} else {
		gtk_tree_model_get_iter_from_string(model, &iter, path);
		gtk_list_store_set(store, &iter, COLOR_SCHEME_LIST_COL_TYPE, snew_type, -1);
		gtk_tree_model_get(model, &iter, COLOR_SCHEME_LIST_COL_CSR_POINTER, &csr, -1);
		csr->type = new_type;
	}
}

void color_scheme_list_color_changed(GtkCellRendererToggle* cell_renderer,
				     gchar* path,
				     gpointer color,
				     gpointer user_data)
{
	GtkTreeView* cs_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	struct color_scheme_rule* csr;
	GdkColor* c = (GdkColor*)color;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, COLOR_SCHEME_LIST_COL_CSR_POINTER, &csr, -1);
	gtk_list_store_set(store, &iter, COLOR_SCHEME_LIST_COL_COLOR, color, -1);

	color_scheme_rule_set_color(csr, c->red / 65535.0, c->green / 65535.0, c->blue / 65535.0);
}

void color_scheme_list_regex_edited(GtkCellRendererText *cell,
				    gchar* path,
				    gchar* new_regex,
				    gpointer user_data)
{
	GtkTreeView* cs_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	struct color_scheme_rule* csr;
	int regex_valid = is_valid_regex(new_regex);
	const char* regex_color = (regex_valid) ? "darkgreen" : "red";

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, COLOR_SCHEME_LIST_COL_CSR_POINTER, &csr, -1);

	if(!regex_valid)
		show_error_message("Not a valid regular expression");

	if(strcmp(csr->regex, new_regex)) {
		if(color_scheme_rule_set_regex(csr, new_regex)) {
			show_error_message("Could not set regex for color scheme rule");
			return;
		}

		gtk_list_store_set(store, &iter, COLOR_SCHEME_LIST_COL_REGEX, csr->regex, -1);
		gtk_list_store_set(store, &iter, COLOR_SCHEME_LIST_COL_REGEX_FOREGROUND, regex_color, -1);
	}
}

void color_scheme_list_init(GtkTreeView* cs_treeview)
{
	GtkCellRenderer* type_renderer;
	GtkCellRenderer* regex_renderer;
	GtkCellRenderer* color_renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;
	GtkListStore *list_store_combo;
	GtkTreeIter iter;

	list_store_combo = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	for(int i = 0; i < COLOR_SCHEME_RULE_TYPE_MAX; i++) {
		gtk_list_store_append(list_store_combo, &iter);
		gtk_list_store_set(list_store_combo, &iter, 0, color_scheme_rule_type_name(i), 1, i, -1);
//		gtk_list_store_set(list_store_combo, &iter, 0, color_scheme_rule_type_name(i), -1);
	}

	type_renderer = gtk_cell_renderer_combo_new();
	g_object_set(G_OBJECT(type_renderer), "model", list_store_combo, "has-entry", FALSE, "editable", TRUE, "text-column", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes("Type", type_renderer, "text", COLOR_SCHEME_LIST_COL_TYPE, NULL);
	gtk_tree_view_append_column(cs_treeview, column);

	regex_renderer = gtk_cell_renderer_text_new();
	g_object_set(regex_renderer, "editable", TRUE, NULL);
	column = gtk_tree_view_column_new_with_attributes("Regex", regex_renderer, "text", COLOR_SCHEME_LIST_COL_REGEX, "foreground", COLOR_SCHEME_LIST_COL_REGEX_FOREGROUND, NULL);
	gtk_tree_view_append_column(cs_treeview, column);

	color_renderer = custom_cell_renderer_color_button_new();
	column = gtk_tree_view_column_new_with_attributes("Color", color_renderer, "color", COLOR_SCHEME_LIST_COL_COLOR, NULL);
	gtk_tree_view_append_column(cs_treeview, column);

	store = gtk_list_store_new(COLOR_SCHEME_LIST_COL_NUM, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_COLOR, G_TYPE_POINTER);

	gtk_tree_view_set_model(cs_treeview, GTK_TREE_MODEL(store));

	g_signal_connect(G_OBJECT(type_renderer),  "edited", G_CALLBACK(color_scheme_list_type_changed), cs_treeview);
	g_signal_connect(G_OBJECT(regex_renderer), "edited", G_CALLBACK(color_scheme_list_regex_edited), cs_treeview);
	g_signal_connect(G_OBJECT(color_renderer), "color-changed", G_CALLBACK(color_scheme_list_color_changed), cs_treeview);

	g_object_unref(store);
	g_object_unref(list_store_combo);
}

void color_scheme_list_fill(GtkTreeView* cs_treeview, struct color_scheme* cs)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	GdkColor color;
	const char* regex_color;

	for(int i = 0; i < cs->num_rules; i++) {
		color.red = cs->rules[i]->color_r * 65535;
		color.green = cs->rules[i]->color_g * 65535;
		color.blue = cs->rules[i]->color_b* 65535;

		if(is_valid_regex(cs->rules[i]->regex))
			regex_color = "darkgreen";
		else
			regex_color = "red";

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   COLOR_SCHEME_LIST_COL_TYPE, color_scheme_rule_type_name(cs->rules[i]->type),
				   COLOR_SCHEME_LIST_COL_REGEX, cs->rules[i]->regex,
				   COLOR_SCHEME_LIST_COL_REGEX_FOREGROUND, regex_color,
				   COLOR_SCHEME_LIST_COL_COLOR, &color,
				   COLOR_SCHEME_LIST_COL_CSR_POINTER, cs->rules[i],
				   -1);
	}
}

void color_scheme_list_clear(GtkTreeView* cs_treeview)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);

	gtk_list_store_clear(store);
}

void color_scheme_list_reset(GtkTreeView* cs_treeview, struct color_scheme* cs)
{
	color_scheme_list_clear(cs_treeview);
	color_scheme_list_fill(cs_treeview, cs);
}

struct color_scheme_rule* color_scheme_list_get_selected(GtkTreeView* cs_treeview)
{
	GtkTreeSelection *selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	struct color_scheme_rule* csr;

	selection = gtk_tree_view_get_selection(cs_treeview);

	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, COLOR_SCHEME_LIST_COL_CSR_POINTER, &csr, -1);
		return csr;
	}

	return NULL;
}

struct color_scheme_rule* color_scheme_list_get(GtkTreeView* cs_treeview, GtkTreePath* path)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkTreeIter iter;
	struct color_scheme_rule* csr;

	if(!gtk_tree_model_get_iter(model, &iter, path))
		return NULL;

	gtk_tree_model_get(model, &iter, COLOR_SCHEME_LIST_COL_CSR_POINTER, &csr, -1);

	return csr;
}

void color_scheme_list_remove(GtkTreeView* cs_treeview, struct color_scheme_rule* csr)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cs_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	struct color_scheme_rule* csriter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_tree_model_get(model, &iter, COLOR_SCHEME_LIST_COL_CSR_POINTER, &csriter, -1);

		if(csriter == csr) {
			gtk_list_store_remove(store, &iter);
			return;
		}
	} while(gtk_tree_model_iter_next(model, &iter));
}
