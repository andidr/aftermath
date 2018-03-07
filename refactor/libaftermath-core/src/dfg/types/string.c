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

#include "string.h"
#include <string.h>

int am_dfg_type_string_to_string(const struct am_dfg_type* t,
				 void* ptr,
				 char** out,
				 int* cst)
{
	*cst = 1;
	*out = ptr;

	return 0;
}

int am_dfg_type_string_from_string(const struct am_dfg_type* t,
				   const char* str,
				   void* out)
{
	char** sout = out;
	char* tmp;

	if(!(tmp = strdup(str)))
		return 1;

	*sout = tmp;

	return 0;
}

int am_dfg_type_string_check_string(const struct am_dfg_type* t, const char* str)
{
	return 1;
}
