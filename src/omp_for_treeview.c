/**
 * Copyright (C) 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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
#include "omp_for_treeview.h"
#include "omp_for.h"
#include "color.h"
#include "cell_renderer_color_button.h"
#include "util.h"
#include "globals.h"
#include "marshal.h"

gint gtk_omp_for_treeview_signals[GTK_OMP_FOR_TREEVIEW_MAX_SIGNALS] = { 0 };

void omp_for_treeview_row_selected(GtkTreeView* omp_treeview, GtkTreePath* path, GtkTreeViewColumn *col, gpointer user_data)
{
	GtkTrace* g = GTK_TRACE(g_trace_widget);
	GtkTreeView* omp_for_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(omp_for_treeview);
	GtkTreeIter iter;
	struct omp_for_chunk_set_part* ofcp;
	int level;

	gchar* path_string = gtk_tree_path_to_string(path);

	gtk_tree_model_get_iter_from_string(model, &iter, path_string);

	g_free(path_string);

	gtk_tree_model_get(model, &iter,
			   OMP_FOR_TREEVIEW_COL_LEVEL, &level,
			   OMP_FOR_TREEVIEW_COL_POINTER, &ofcp,
			   -1);

	if(level != 3)
		return;

	if(ofcp && (filter_has_ofcp(g->filter, ofcp))) {
		g_signal_emit(g_omp_for_treeview_type,
			      gtk_omp_for_treeview_signals[GTK_OMP_FOR_TREEVIEW_UPDATE_HIGHLIGHTED_PART],
			      0, ofcp);
	}
}

void omp_for_treeview_color_changed(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer color, gpointer user_data)
{
	GtkTreeView* omp_for_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(omp_for_treeview);
	GtkTreeIter iter;
	GtkTreeStore* store = GTK_TREE_STORE(model);
	int path_to_object[4] = {-1, -1, -1, -1};

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_tree_store_set(store, &iter, OMP_FOR_TREEVIEW_COL_COLOR, color, -1);

	sscanf(path, "%d:%d:%d:%d",
	       &path_to_object[0],
	       &path_to_object[1],
	       &path_to_object[2],
	       &path_to_object[3]);

	if(path_to_object[1] == -1) {
		struct omp_for * of;
		gtk_tree_model_get(model, &iter,
				OMP_FOR_TREEVIEW_COL_POINTER, &of,
				-1);
		of->color_r = ((GdkColor*)color)->red / 65536.0;
		of->color_g = ((GdkColor*)color)->green / 65536.0;
		of->color_b = ((GdkColor*)color)->blue / 65536.0;
	} else if(path_to_object[2] == -1) {
		struct omp_for_instance * ofi;
		gtk_tree_model_get(model, &iter,
				OMP_FOR_TREEVIEW_COL_POINTER, &ofi,
				-1);
		ofi->color_r = ((GdkColor*)color)->red / 65536.0;
		ofi->color_g = ((GdkColor*)color)->green / 65536.0;
		ofi->color_b = ((GdkColor*)color)->blue / 65536.0;
	} else if(path_to_object[3] == -1) {
		struct omp_for_chunk_set * ofc;
		gtk_tree_model_get(model, &iter,
				OMP_FOR_TREEVIEW_COL_POINTER, &ofc,
				-1);
		ofc->color_r = ((GdkColor*)color)->red / 65536.0;
		ofc->color_g = ((GdkColor*)color)->green / 65536.0;
		ofc->color_b = ((GdkColor*)color)->blue / 65536.0;
	} else {
		struct omp_for_chunk_set_part * ofcp;
		gtk_tree_model_get(model, &iter,
				OMP_FOR_TREEVIEW_COL_POINTER, &ofcp,
				-1);
		ofcp->color_r = ((GdkColor*)color)->red / 65536.0;
		ofcp->color_g = ((GdkColor*)color)->green / 65536.0;
		ofcp->color_b = ((GdkColor*)color)->blue / 65536.0;
	}

	gtk_widget_queue_draw(g_trace_widget);
}

void omp_for_treeview_toggle_children(GtkTreeModel* model, GtkTreeStore* store, GtkTreeIter* iter, gboolean current_state, int level)
{
		do {
			gtk_tree_store_set(store, &iter[level], OMP_FOR_TREEVIEW_COL_FILTER, !current_state, -1);
			if(level < 3) {
				if(gtk_tree_model_iter_children(model, &iter[level + 1], &iter[level]) == FALSE)
					continue;
				omp_for_treeview_toggle_children(model, store, iter, current_state, level + 1);
			}
		} while(gtk_tree_model_iter_next(model, &iter[level]));
}

void omp_for_treeview_toggle(GtkCellRendererToggle* cell_renderer, gchar* path, gpointer user_data)
{
	GtkTreeView* omp_for_treeview = user_data;
	GtkTreeModel* model = gtk_tree_view_get_model(omp_for_treeview);
	GtkTreeIter iter[4];

	GtkTreeStore* store = GTK_TREE_STORE(model);
	gboolean current_state;

	gtk_tree_model_get_iter_from_string(model, &iter[0], path);
	gtk_tree_model_get(model, &iter[0], OMP_FOR_TREEVIEW_COL_FILTER, &current_state, -1);
	gtk_tree_store_set(store, &iter[0], OMP_FOR_TREEVIEW_COL_FILTER, !current_state, -1);

	if(gtk_tree_model_iter_children(model, &iter[1], &iter[0]) == FALSE)
		return;

	omp_for_treeview_toggle_children(model, store, iter, current_state, 1);
}

GtkWidget* omp_for_treeview_init(GtkTreeView* omp_for_treeview)
{
	GtkCellRenderer* renderer;
	GtkTreeViewColumn* column;
	GtkOmpTreeViewType *omp_tt = gtk_type_new(gtk_omp_treeview_get_type());

	GtkTreeStore* store = gtk_tree_store_new(OMP_FOR_TREEVIEW_COL_NUM,
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

	g_signal_connect((GtkWidget *)(omp_for_treeview), "row-activated",
			 G_CALLBACK(omp_for_treeview_row_selected), omp_for_treeview);

	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "active", OMP_FOR_TREEVIEW_COL_FILTER, NULL);
	gtk_cell_renderer_toggle_set_activatable(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(omp_for_treeview_toggle), omp_for_treeview);

	renderer = custom_cell_renderer_color_button_new();
	column = gtk_tree_view_column_new_with_attributes("Color", renderer, "color", OMP_FOR_TREEVIEW_COL_COLOR, NULL);
	gtk_tree_view_append_column(omp_for_treeview, column);
	g_signal_connect(G_OBJECT(renderer), "color-changed", G_CALLBACK(omp_for_treeview_color_changed), GTK_TREE_VIEW(omp_for_treeview));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", OMP_FOR_TREEVIEW_COL_ADDR, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Symbol", renderer, "text", OMP_FOR_TREEVIEW_COL_SYMBOL, NULL);
	gtk_tree_view_column_set_fixed_width(column, 100);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Start", renderer, "text", OMP_FOR_TREEVIEW_COL_START, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("End", renderer, "text", OMP_FOR_TREEVIEW_COL_END, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("CPU", renderer, "text", OMP_FOR_TREEVIEW_COL_CPU, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Source file", renderer, "text", OMP_FOR_TREEVIEW_COL_SOURCE_FILE, NULL);
	gtk_tree_view_column_set_fixed_width(column, 100);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	column = gtk_tree_view_column_new_with_attributes("Line", renderer, "text", OMP_FOR_TREEVIEW_COL_SOURCE_LINE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(omp_for_treeview, column);

	gtk_tree_view_set_model(omp_for_treeview, GTK_TREE_MODEL(store));
	g_object_unref(store);

	return GTK_WIDGET(omp_tt);
}

GtkType gtk_omp_treeview_get_type(void)
{
	static GtkType gtk_omp_treeview_type = 0;

	if (!gtk_omp_treeview_type) {
		static const GtkTypeInfo gtk_omp_treeview_type_info = {
			"GtkOmpTreeView",
			sizeof(GtkOmpTreeViewType),
			sizeof(GtkOmpTreeViewClass),
			(GtkClassInitFunc) gtk_omp_treeview_class_init,
			(GtkObjectInitFunc) gtk_omp_treeview_type_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		gtk_omp_treeview_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_omp_treeview_type_info);
	}

	return gtk_omp_treeview_type;
}

void gtk_omp_treeview_type_destroy(GtkObject *object)
{
	GtkOmpTreeViewClass *class;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_OMP_TREEVIEW(object));

	class = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(class)->destroy) {
		(* GTK_OBJECT_CLASS(class)->destroy)(object);
	}
}

void gtk_omp_treeview_type_init(GtkOmpTreeViewType *treeviewtype)
{
}

void gtk_omp_treeview_class_init(GtkOmpTreeViewClass *class)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) class;

	gtk_omp_for_treeview_signals[GTK_OMP_FOR_TREEVIEW_UPDATE_HIGHLIGHTED_PART] =
                g_signal_new("omp-update-highlighted-part", G_OBJECT_CLASS_TYPE(object_class),
                             GTK_RUN_FIRST,
                             G_STRUCT_OFFSET(GtkOmpTreeViewClass, bounds_changed),
                             NULL, NULL,
                             g_cclosure_user_marshal_VOID__POINTER,
                             G_TYPE_NONE, 1,
                             G_TYPE_POINTER);
}

void omp_for_add_chunk_set_parts(GtkTreeStore* store, struct omp_for_chunk_set* ofc, GtkTreeIter* iter_chunk_set, GdkColor* color)
{
	GtkTreeIter iter_part;
	struct list_head* l_iter_part;
	struct omp_for_chunk_set_part* ofcp;
	char tmp[20];
	char buff_start[20];
	char buff_end[20];
	char cpu[4];

	list_for_each(l_iter_part, &ofc->chunk_set_parts) {
		ofcp = list_entry(l_iter_part, struct omp_for_chunk_set_part, list);

		pretty_print_cycles(tmp, sizeof(buff_start), ofcp->start);
		snprintf(buff_start, sizeof(buff_start), "%s%s", tmp, "cycles");
		pretty_print_cycles(tmp, sizeof(buff_end), ofcp->end);
		snprintf(buff_end, sizeof(buff_end), "%s%s", tmp, "cycles");
		snprintf(cpu, sizeof(cpu), "%d", ofcp->cpu);

		gtk_tree_store_append(store, &iter_part, iter_chunk_set);
		gtk_tree_store_set(store, &iter_part,
				OMP_FOR_TREEVIEW_COL_FILTER, TRUE,
				OMP_FOR_TREEVIEW_COL_COLOR, color,
				OMP_FOR_TREEVIEW_COL_ADDR, "",
				OMP_FOR_TREEVIEW_COL_START, buff_start,
				OMP_FOR_TREEVIEW_COL_END, buff_end,
				OMP_FOR_TREEVIEW_COL_CPU, cpu,
				OMP_FOR_TREEVIEW_COL_POINTER, ofcp,
				OMP_FOR_TREEVIEW_COL_LEVEL, 3,
				-1);
	}
}

void omp_for_add_chunk_sets(GtkTreeStore* store, struct omp_for_instance* ofi, GtkTreeIter* iter_fori, GdkColor* color)
{
	GtkTreeIter iter_chunk_set;
	struct list_head* l_iter_chunk_set;
	struct omp_for_chunk_set* ofc;
	char buff_start[20];
	char buff_end[20];

	list_for_each(l_iter_chunk_set, &ofi->for_chunk_sets) {
		ofc = list_entry(l_iter_chunk_set, struct omp_for_chunk_set, list);

		if(ofi->flags && OMP_FOR_SIGNED_ITERATION_SPACE) {
			snprintf(buff_start, sizeof(buff_start), "%"PRId64, ofc->iter_start);
			snprintf(buff_end, sizeof(buff_end), "%"PRId64, ofc->iter_end);
		} else {
			snprintf(buff_start, sizeof(buff_start), "%"PRIu64, ofc->iter_start);
			snprintf(buff_end, sizeof(buff_end), "%"PRIu64, ofc->iter_end);
		}

		gtk_tree_store_append(store, &iter_chunk_set, iter_fori);
		gtk_tree_store_set(store, &iter_chunk_set,
				OMP_FOR_TREEVIEW_COL_FILTER, TRUE,
				OMP_FOR_TREEVIEW_COL_COLOR, color,
				OMP_FOR_TREEVIEW_COL_ADDR, "",
				OMP_FOR_TREEVIEW_COL_START, buff_start,
				OMP_FOR_TREEVIEW_COL_END, buff_end,
				OMP_FOR_TREEVIEW_COL_CPU, "",
				OMP_FOR_TREEVIEW_COL_POINTER, ofc,
				OMP_FOR_TREEVIEW_COL_LEVEL, 2,
				-1);
		omp_for_add_chunk_set_parts(store, ofc, &iter_chunk_set, color);
	}
}

void omp_for_add_instances(GtkTreeStore* store, struct omp_for* of, GtkTreeIter* iter_for)
{
	GtkTreeIter iter_fori;
	GdkColor color;
	struct list_head* l_iter_fori;
	struct omp_for_instance* ofi;
	char buff_start[20];
	char buff_end[20];

	list_for_each(l_iter_fori, &(of->for_instances)) {
		ofi = list_entry(l_iter_fori, struct omp_for_instance, list);
		if(ofi->flags && OMP_FOR_SIGNED_ITERATION_SPACE) {
			snprintf(buff_start, sizeof(buff_start), "%"PRId64, ofi->iter_start);
			snprintf(buff_end, sizeof(buff_end), "%"PRId64, ofi->iter_end);
		} else {
			snprintf(buff_start, sizeof(buff_start), "%"PRIu64, ofi->iter_start);
			snprintf(buff_end, sizeof(buff_end), "%"PRIu64, ofi->iter_end);
		}

		color.red = ofi->color_r * 65535.0;
		color.green = ofi->color_g * 65535.0;
		color.blue = ofi->color_b * 65535.0;
		gtk_tree_store_append(store, &iter_fori, iter_for);
		gtk_tree_store_set(store, &iter_fori,
				OMP_FOR_TREEVIEW_COL_FILTER, TRUE,
				OMP_FOR_TREEVIEW_COL_COLOR, &color,
				OMP_FOR_TREEVIEW_COL_ADDR, "",
				OMP_FOR_TREEVIEW_COL_START, buff_start,
				OMP_FOR_TREEVIEW_COL_END, buff_end,
				OMP_FOR_TREEVIEW_COL_CPU, "",
				OMP_FOR_TREEVIEW_COL_POINTER, ofi,
				OMP_FOR_TREEVIEW_COL_LEVEL, 1,
				-1);

		omp_for_add_chunk_sets(store, ofi, &iter_fori, &color);
	}
}

void omp_for_add_loops(GtkTreeStore* store, struct omp_for* omp_fors, int num_omp_fors)
{
	GtkTreeIter iter_for;
	char buff_addr[20];
	char line[20];
	GdkColor color;

	for(int i = 0; i < num_omp_fors; i++) {
		snprintf(buff_addr, sizeof(buff_addr), "0x%"PRIx64, omp_fors[i].addr);
		snprintf(line, sizeof(line), "%u", omp_fors[i].source_line);
		color.red = omp_fors[i].color_r * 65535.0;
		color.green = omp_fors[i].color_g * 65535.0;
		color.blue = omp_fors[i].color_b * 65535.0;
		gtk_tree_store_append(store, &iter_for, NULL);
		gtk_tree_store_set(store, &iter_for,
				   OMP_FOR_TREEVIEW_COL_FILTER, TRUE,
				   OMP_FOR_TREEVIEW_COL_COLOR, &color,
				   OMP_FOR_TREEVIEW_COL_ADDR, buff_addr,
				   OMP_FOR_TREEVIEW_COL_SYMBOL, (omp_fors[i].symbol_name == NULL)?
							     "debug symbol not found" :
							     omp_fors[i].symbol_name,
				   OMP_FOR_TREEVIEW_COL_START, "",
				   OMP_FOR_TREEVIEW_COL_END, "",
				   OMP_FOR_TREEVIEW_COL_CPU, "",
				   OMP_FOR_TREEVIEW_COL_SOURCE_FILE, (omp_fors[i].source_filename == NULL)?
							          "debug symbol not found" :
							          omp_fors[i].source_filename,
				   OMP_FOR_TREEVIEW_COL_SOURCE_LINE, (omp_fors[i].source_line == -1)?
								  "debug symbol not found" :
								  line,
				   OMP_FOR_TREEVIEW_COL_POINTER, &omp_fors[i],
				   OMP_FOR_TREEVIEW_COL_LEVEL, 0,
				   -1);
		omp_for_add_instances(store, &omp_fors[i], &iter_for);
	}
}

void omp_for_treeview_fill(GtkTreeView* omp_for_treeview, struct omp_for* omp_fors, int num_omp_fors)
{
	GtkTreeModel* model = gtk_tree_view_get_model(omp_for_treeview);
	GtkTreeStore* store = GTK_TREE_STORE(model);

	omp_for_add_loops(store, omp_fors, num_omp_fors);
}

int omp_for_treeview_add_to_filter(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer filter)
{
	gboolean current_state;
	int level;
	struct omp_for_chunk_set_part * ofcp;

	gtk_tree_model_get(model, iter,
			   OMP_FOR_TREEVIEW_COL_FILTER, &current_state,
			   OMP_FOR_TREEVIEW_COL_LEVEL, &level,
			   OMP_FOR_TREEVIEW_COL_POINTER, &ofcp,
			   -1);

	if(level == 3 && current_state)
		filter_add_ofcp((struct filter*)filter, ofcp);

	return FALSE;
}

void omp_for_treeview_build_filter(GtkTreeView* omp_for_treeview, struct filter* filter)
{
	GtkTreeModel* model = gtk_tree_view_get_model(omp_for_treeview);

	gtk_tree_model_foreach(model, omp_for_treeview_add_to_filter, filter);

	filter_sort_ofcps(filter);
	filter_set_ofcp_filtering(filter, 1);
}
