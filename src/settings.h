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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdlib.h>
#include <glib.h>

struct settings {
	int use_external_editor;
	char* external_editor_command;
};

static inline void settings_init(struct settings* s)
{
	s->use_external_editor = 0;
	s->external_editor_command = NULL;
}

static inline void settings_destroy(struct settings* s)
{
	g_free(s->external_editor_command);
}

int settings_set_string(char** str, const char* val);
int read_user_settings(struct settings* s);
int write_user_settings(struct settings* s);

#endif
