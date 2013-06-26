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

#include <stdio.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <malloc.h>
#include "glade_extras.h"
#include "trace_widget.h"
#include "paraver.h"
#include "globals.h"
#include "signals.h"
#include "detect.h"
#include "dialogs.h"
#include "task_list.h"

int main(int argc, char** argv)
{
	GladeXML *xml;
	char* tracefile = NULL;

	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s trace_file\n", argv[0]);
		return 1;
	}

	gtk_init(&argc, &argv);
	tracefile = argv[1];

	enum trace_format format;
	if(detect_trace_format(tracefile, &format)) {
		show_error_message("Could not open file %s", tracefile);
		return 1;
	}

	if(format == TRACE_FORMAT_UNKNOWN) {
		show_error_message("Cannot detect trace format of file %s", tracefile);
		return 1;
	} else if(format == TRACE_FORMAT_OSTV) {
		if(read_trace_sample_file(&g_mes, tracefile) != 0) {
			show_error_message("Cannot read samples from %s", tracefile);
			return 1;
		}
	} else if(format == TRACE_FORMAT_PARAVER) {
		if(read_paraver_samples(&g_mes, tracefile) != 0) {
			show_error_message("Cannot read samples from %s", tracefile);
			return 1;
		}
	}

	xml = glade_xml_new(DATA_PATH "/ostv.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, toplevel_window);
	IMPORT_GLADE_WIDGET(xml, graph_box);
	IMPORT_GLADE_WIDGET(xml, scroll_bar);
	IMPORT_GLADE_WIDGET(xml, task_treeview);

	g_trace_widget = gtk_trace_new(&g_mes);
	gtk_container_add(GTK_CONTAINER(graph_box), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "bounds-changed", G_CALLBACK(trace_bounds_changed), g_trace_widget);

	g_scroll_bar = scroll_bar;
	g_task_treeview = task_treeview;

	task_list_init(GTK_TREE_VIEW(g_task_treeview));
	task_list_fill(GTK_TREE_VIEW(g_task_treeview), g_mes.tasks, g_mes.num_tasks);

	filter_init(&g_filter);

	gtk_widget_show_all(toplevel_window);

	reset_zoom();

	gtk_main();

	multi_event_set_destroy(&g_mes);
	filter_destroy(&g_filter);

	return 0;
}
