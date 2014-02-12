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

#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include "multi_event_set.h"
#include "filter.h"

int export_task_graph(const char* outfile, struct multi_event_set* mes, struct filter* f, int64_t start, int64_t end);
int export_task_graph_selected_texec(const char* outfile, struct multi_event_set* mes, struct single_event* texec_start, unsigned int depth_down);

#endif