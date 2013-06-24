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

int main(int argc, char** argv)
{
	GladeXML *xml;
	char* tracefile = NULL;

	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s trace_file\n", argv[0]);
		return 1;
	}

	tracefile = argv[1];

	printf("tf = %s, tf = %s\n", argv[1], tracefile);

	gtk_init(&argc, &argv);

	xml = glade_xml_new(DATA_PATH "/ostv.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, toplevel_window);
	IMPORT_GLADE_WIDGET(xml, graph_box);
	IMPORT_GLADE_WIDGET(xml, scroll_bar);

	read_paraver_samples(&g_mes, tracefile);

	g_trace_widget = gtk_trace_new(&g_mes);
	gtk_container_add(GTK_CONTAINER(graph_box), g_trace_widget);
	g_signal_connect(G_OBJECT(g_trace_widget), "bounds-changed", G_CALLBACK(trace_bounds_changed), g_trace_widget);

	g_scroll_bar = scroll_bar;

	gtk_widget_show_all(toplevel_window);

	gtk_main();

	multi_event_set_destroy(&g_mes);

	return 0;
}
