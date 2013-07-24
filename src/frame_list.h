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

#ifndef FRAME_LIST_H
#define FRAME_LIST_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"
#include "frame.h"

enum frame_list_columns {
	FRAME_LIST_COL_FILTER = 0,
	FRAME_LIST_COL_ADDR,
	FRAME_LIST_COL_NUM_STEALS,
	FRAME_LIST_COL_NUM_PUSHES,
	FRAME_LIST_COL_FRAME_POINTER,
	FRAME_LIST_COL_NUM
};

void frame_list_init(GtkTreeView* frame_treeview);
void frame_list_fill(GtkTreeView* frame_treeview, struct frame* frames, int num_frames);
void frame_list_build_filter(GtkTreeView* frame_treeview, struct filter* filter);

void frame_list_check_all(GtkTreeView* frame_treeview);
void frame_list_uncheck_all(GtkTreeView* frame_treeview);

#endif
