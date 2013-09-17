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

#include "debug.h"
#include <stdio.h>
#include <inttypes.h>

char *strdup(const char *s);

int debug_read_task_symbols(const char* filename, struct multi_event_set* mes)
{
	int ret = 1;
	char command[FILENAME_MAX];
	uint64_t curr_addr;
	char curr_sym_name[256];
	char curr_file_name[256];
	int curr_line_number;
	char curr_sym_type;
	FILE* fp;
	struct task key;
	struct task* t;

	snprintf(command, sizeof(command), "ostv-debug-syms \"%s\"", filename);

	if(!(fp = popen(command, "r")))
		goto out;

	while(!feof(fp)) {
		char c = ' ';
		fscanf(fp, "%"PRIx64" %c %s %s %d", &curr_addr, &curr_sym_type, curr_sym_name, curr_file_name, &curr_line_number);

		key.addr = curr_addr;
		if((t = multi_event_set_find_task(mes, &key))) {
			if(!(t->source_filename = strdup(curr_file_name)))
				goto out_fp;

			if(!(t->symbol_name = strdup(curr_sym_name)))
				goto out_fp;

			t->source_line = curr_line_number;
		}

		while(c != '\n' && !feof(fp))
			fscanf(fp, "%c", &c);
	}

	ret = 0;

out_fp:
	ret = pclose(fp);
out:
	return ret;
}
