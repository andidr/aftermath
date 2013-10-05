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

#include "cpu_list.h"
#include "page.h"
#include <inttypes.h>

void cpu_list_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* cpu_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(cpu_treeview);
	GtkTreeIter iter;
	GtkListStore* store = GTK_LIST_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_model_get(model, &iter, CPU_LIST_COL_FILTER, &current_state, -1);
	gtk_list_store_set(store, &iter, CPU_LIST_COL_FILTER, !current_state, -1);
}

void cpu_list_init(GtkTreeView* cpu_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkListStore* store;

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", CPU_LIST_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(cpu_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(cpu_list_toggle), cpu_treeview);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("CPU", renderer, "text", CPU_LIST_COL_CPU, NULL);
	gtk_tree_view_append_column(cpu_treeview, column);

	store = gtk_list_store_new(CPU_LIST_COL_NUM, G_TYPE_BOOLEAN, G_TYPE_STRING);

	gtk_tree_view_set_model(cpu_treeview, GTK_TREE_MODEL(store));

	g_object_unref(store);
}

void cpu_list_fill(GtkTreeView* cpu_treeview, int max_cpu)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cpu_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;
	char buff_cpu[16];

	for(int i = 0; i <= max_cpu; i++) {
		snprintf(buff_cpu, sizeof(buff_cpu), "CPU %d", i);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   CPU_LIST_COL_FILTER, FALSE,
				   CPU_LIST_COL_CPU, buff_cpu,
				   -1);
	}
}

void cpu_list_build_bitvector(GtkTreeView* cpu_treeview, struct bitvector* bv)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cpu_treeview);
	GtkTreeIter iter;
	gboolean current_state;
	int cpu = 0;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	bitvector_clear(bv);

	do {
		gtk_tree_model_get(model, &iter, CPU_LIST_COL_FILTER, &current_state, -1);

		if(current_state)
			bitvector_set_bit(bv, cpu);

		cpu++;
	} while(gtk_tree_model_iter_next(model, &iter));
}

void cpu_list_set_status_all(GtkTreeView* cpu_treeview, gboolean status)
{
	GtkTreeModel* model = gtk_tree_view_get_model(cpu_treeview);
	GtkListStore* store = GTK_LIST_STORE(model);
	GtkTreeIter iter;

	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		gtk_list_store_set(store, &iter, CPU_LIST_COL_FILTER, status, -1);
	} while(gtk_tree_model_iter_next(model, &iter));
}

void cpu_list_check_all(GtkTreeView* cpu_treeview)
{
	cpu_list_set_status_all(cpu_treeview, TRUE);
}

void cpu_list_uncheck_all(GtkTreeView* cpu_treeview)
{
	cpu_list_set_status_all(cpu_treeview, FALSE);
}
