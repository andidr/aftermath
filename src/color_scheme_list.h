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

#ifndef COLOR_SCHEME_LIST_H
#define COLOR_SCHEME_LIST_H

#include <gtk/gtk.h>
#include "color_scheme.h"

enum color_scheme_list_columns {
	COLOR_SCHEME_LIST_COL_TYPE = 0,
	COLOR_SCHEME_LIST_COL_REGEX,
	COLOR_SCHEME_LIST_COL_REGEX_FOREGROUND,
	COLOR_SCHEME_LIST_COL_COLOR,
	COLOR_SCHEME_LIST_COL_CSR_POINTER,
	COLOR_SCHEME_LIST_COL_NUM
};

void color_scheme_list_init(GtkTreeView* cs_treeview);
void color_scheme_list_fill(GtkTreeView* cs_treeview, struct color_scheme* cs);
void color_scheme_list_reset(GtkTreeView* cs_treeview, struct color_scheme* cs);
void color_scheme_list_clear(GtkTreeView* cs_treeview);
void color_scheme_list_remove(GtkTreeView* cs_treeview, struct color_scheme_rule* csr);
struct color_scheme_rule* color_scheme_list_get_selected(GtkTreeView* cs_treeview);
struct color_scheme_rule* color_scheme_list_get(GtkTreeView* cs_treeview, GtkTreePath* path);

#endif
