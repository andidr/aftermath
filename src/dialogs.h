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

#ifndef DIALOGS_H
#define DIALOGS_H

#include <gtk/gtk.h>
#include "settings.h"

struct progress_window_widgets {
	GtkWindow* window;
	GtkProgressBar* progressbar;
	GtkLabel* label_trace_bytes;
	GtkLabel* label_bytes_loaded;
	GtkLabel* label_seconds_remaining;
	GtkLabel* label_throughput;
};

void show_error_message(char* format, ...);
int show_goto_dialog(double start, double end, double curr_value, double* time);
void show_about_dialog(void);
int show_settings_dialog(struct settings* s);
void show_progress_window_persistent(struct progress_window_widgets* widgets);

#endif
