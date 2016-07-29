/**
 * Copyright (C) 2016 Andi Drebes <andi.drebes@lip6.fr>
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

#include "gtk_extras.h"

/* For each direct or indirect descendant of a widget and the widget itseld the
 * callback function func is called with the descendant as the first argument
 * and the data pointer as the second.
 */
void gtk_extra_for_each_descendant(GtkWidget* widget,
				   void (*func)(GtkWidget* widget, void* data),
				   void* data)
{
	GtkWidget* child;
	GList* iter;

	if(!widget)
		return;

	func(widget, data);

	if(GTK_IS_BIN(widget)) {
		child = gtk_bin_get_child(GTK_BIN(widget));
		gtk_extra_for_each_descendant(child, func, data);
	} else if(GTK_IS_CONTAINER(widget)) {
		for(iter = gtk_container_get_children(GTK_CONTAINER(widget));
		    iter;
		    iter = iter->next)
		{
			child = iter->data;
			gtk_extra_for_each_descendant(child, func, data);
		}
	}
}
