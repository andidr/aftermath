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

#ifndef COLOR_SCHEME_SET_LIST_H
#define COLOR_SCHEME_SET_LIST_H

#include <gtk/gtk.h>
#include "color_scheme.h"

enum color_scheme_set_list_columns {
	COLOR_SCHEME_SET_LIST_COL_NAME = 0,
	COLOR_SCHEME_SET_LIST_COL_CS_POINTER,
	COLOR_SCHEME_SET_LIST_COL_NUM
};

void color_scheme_set_list_init(GtkTreeView* css_treeview);
void color_scheme_set_list_fill(GtkTreeView* css_treeview, struct color_scheme_set* css);
void color_scheme_set_list_clear(GtkTreeView* css_treeview);
void color_scheme_set_list_reset(GtkTreeView* css_treeview, struct color_scheme_set* css);
void color_scheme_set_list_remove(GtkTreeView* css_treeview, struct color_scheme* cs);
struct color_scheme* color_scheme_set_list_get_selected(GtkTreeView* css_treeview);
struct color_scheme* color_scheme_set_list_get(GtkTreeView* css_treeview, GtkTreePath* path);

#endif
