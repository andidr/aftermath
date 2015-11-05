/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr> + 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef STATE_LIST_H
#define STATE_LIST_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"
#include "state_description.h"

enum state_list_columns {
	STATE_LIST_COL_COLOR = 0,
	STATE_LIST_COL_NAME,
	STATE_LIST_COL_PER,
	STATE_LIST_COL_PAR,
        STATE_LIST_COL_NUM
};

void state_list_init(GtkTreeView* state_treeview);
void state_list_append(GtkTreeView* state_treeview, struct state_description* state, int init);
void state_list_fill(GtkTreeView* state_treeview, struct state_description* states, int num_states);
void state_list_fill_name(GtkTreeView* state_treeview, struct state_description* states, int num_states);
void state_list_clear(GtkTreeView* state_treeview);

#endif
