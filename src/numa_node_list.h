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

#ifndef NUMA_NODE_LIST_H
#define NUMA_NODE_LIST_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"

enum numa_node_list_columns {
	NUMA_NODE_LIST_COL_FILTER = 0,
	NUMA_NODE_LIST_COL_NUMA_NODE,
	NUMA_NODE_LIST_COL_NUM
};

void numa_node_list_init(GtkTreeView* numa_node_treeview);
void numa_node_list_fill(GtkTreeView* numa_node_treeview, int max_numa_node_id);
void numa_node_list_build_frame_filter(GtkTreeView* numa_node_treeview, struct filter* filter);
void numa_node_list_build_comm_filter(GtkTreeView* numa_node_treeview, struct filter* filter);
void numa_node_list_build_writes_to_numa_nodes_filter(GtkTreeView* numa_node_treeview, struct filter* filter);

void numa_node_list_check_all(GtkTreeView* numa_node_treeview);
void numa_node_list_uncheck_all(GtkTreeView* numa_node_treeview);

#endif
