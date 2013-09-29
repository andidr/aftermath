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

#ifndef CPU_LIST_H
#define CPU_LIST_H

#include <gtk/gtk.h>
#include "events.h"
#include "filter.h"
#include "bitvector.h"

enum cpu_list_columns {
	CPU_LIST_COL_FILTER = 0,
	CPU_LIST_COL_CPU,
	CPU_LIST_COL_NUM
};

void cpu_list_init(GtkTreeView* cpu_treeview);
void cpu_list_fill(GtkTreeView* cpu_treeview, int max_cpu);
void cpu_list_build_bitvector(GtkTreeView* cpu_treeview, struct bitvector* bv);

void cpu_list_check_all(GtkTreeView* cpu_treeview);
void cpu_list_uncheck_all(GtkTreeView* cpu_treeview);

#endif
