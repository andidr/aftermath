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

#ifndef DETECT_H
#define DETECT_H

enum trace_format {
	TRACE_FORMAT_OSTV = 0,
	TRACE_FORMAT_PARAVER,
	TRACE_FORMAT_UNKNOWN
};

/* Detect the format used in a trace file specified by "file" */
int detect_trace_format(const char* file, enum trace_format* out);

#endif
