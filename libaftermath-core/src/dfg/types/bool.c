/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include "bool.h"
#include <aftermath/core/ansi_extras.h>
#include <stdio.h>

int am_dfg_type_bool_to_string(const struct am_dfg_type* t,
			       void* ptr,
			       char** out,
			       int* cst)
{
	char* ret;
	int* b = ptr;

	ret = (*b) ? strdup("true") : strdup("false");

	if(!ret)
		return 1;

	*out = ret;
	*cst = 0;

	return 0;
}

int am_dfg_type_bool_from_string(const struct am_dfg_type* t,
				 const char* str,
				 void* out)
{
	int* b = out;

	if(strcmp(str, "true") == 0) {
		*b = 1;
		return 0;
	} else if(strcmp(str, "false") == 0) {
		*b = 0;
		return 0;
	}

	return 1;
}

int am_dfg_type_bool_check_string(const struct am_dfg_type* t,
				  const char* str)
{
	return strcmp(str, "true") == 0 || strcmp(str, "false") == 0;
}
