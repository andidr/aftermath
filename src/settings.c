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

#include "settings.h"
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

char *strdup(const char *s);

static char* default_settings =
	"[settings]\n"
	"use_external_editor = false\n";

int write_settings(const char* filename, const char* settings, int length)
{
	FILE* fp;

	if(!(fp = fopen(filename, "w+")))
		return 1;

	if(fwrite(settings, length, 1, fp) != 1) {
		fclose(fp);
		return 1;
	}

	fclose(fp);
	return 0;
}

int init_settings_if_not_exists(const char* filename)
{
	struct stat st;

	if(stat(filename, &st) == -1) {
		if(errno == ENOENT) {
			return write_settings(filename, default_settings, strlen(default_settings));
		} else {
			return 1;
		}
	}

	return 0;
}

int write_user_settings(struct settings* s)
{
	GKeyFile* keyfile;
	GKeyFileFlags flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
	GError* error = NULL;
	char conf_filename[FILENAME_MAX];
	int ret = 1;
	gchar* conf_data;
	gsize conf_data_length;

	char* home_dir = getenv("HOME");

	if(!home_dir)
		goto out;

	snprintf(conf_filename, sizeof(conf_filename), "%s/.aftermath-settings", home_dir);

	if(init_settings_if_not_exists(conf_filename) != 0)
		goto out;

	if(!(keyfile = g_key_file_new()))
		goto out;

	if(!g_key_file_load_from_file(keyfile, conf_filename, flags, &error))
		goto out_kf;

	g_key_file_set_boolean(keyfile, "settings", "use_external_editor", s->use_external_editor);

	if(s->external_editor_command != NULL)
		g_key_file_set_string(keyfile, "settings", "external_editor_command", s->external_editor_command);

	if(!(conf_data = g_key_file_to_data(keyfile, &conf_data_length, &error)))
		goto out_kf;

	if(write_settings(conf_filename, conf_data, conf_data_length) != 0)
		goto out_kf;

	ret = 0;

out_kf:
	g_key_file_free(keyfile);
out:
	return ret;
}

int read_user_settings(struct settings* s)
{
	GKeyFile* keyfile;
	GKeyFileFlags flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
	GError* error = NULL;
	char conf_filename[FILENAME_MAX];
	int ret = 1;

	char* home_dir = getenv("HOME");

	if(!home_dir)
		goto out;

	snprintf(conf_filename, sizeof(conf_filename), "%s/.aftermath-settings", home_dir);

	if(init_settings_if_not_exists(conf_filename) != 0)
		goto out;

	if(!(keyfile = g_key_file_new()))
		goto out;

	if(!g_key_file_load_from_file(keyfile, conf_filename, flags, &error))
		goto out_kf;

	error = NULL;
	s->use_external_editor = g_key_file_get_boolean(keyfile, "settings", "use_external_editor", &error);

	error = NULL;
	s->external_editor_command = g_key_file_get_string(keyfile, "settings", "external_editor_command", &error);

	ret = 0;

out_kf:
	g_key_file_free(keyfile);
out:
	return ret;
}

int settings_set_string(char** str, const char* val)
{
	if(*str)
		g_free(*str);

	if(!(*str = g_malloc(strlen(val)+1)))
		return 1;

	strcpy(*str, val);

	return 0;
}
