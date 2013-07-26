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

#ifndef GLADE_EXTRAS_H
#define GLADE_EXTRAS_H

#include <glade/glade.h>

/**
 * Declares a pointer to widget whose name is given by "name" assigns it to
 * the widget instance with the same name found in the widget list of a GladeXML
 * UI loader.
 */
#define IMPORT_GLADE_WIDGET(xml, name) \
	GtkWidget* name  = glade_xml_get_widget(xml, #name)

/**
 * Retrieves the widget whose name is given by "name" from the glade XML
 * object and assigns it to the variable specified by var_name.
 */
#define IMPORT_GLADE_WIDGET_ASSIGN(xml, var_name, name)	\
	var_name  = glade_xml_get_widget(xml, #name)

/**
 * Retrieves the widget whose name is given by "name" from the glade XML
 * object and assigns it to the struct member of having the same name of
 * the structure instance specified by struct_ref.
 */
#define IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, struct_ref, name)	\
	(struct_ref)->name  = glade_xml_get_widget(xml, #name)

#endif
