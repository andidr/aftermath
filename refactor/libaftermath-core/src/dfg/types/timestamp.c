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

#include "timestamp.h"
#include <stdio.h>

int am_dfg_type_timestamp_to_string(const struct am_dfg_type* t,
				    void* ptr,
				    char** out,
				    int* cst)
{
	char* ret;
	am_timestamp_t* ts = ptr;

	if(!(ret = malloc(AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 1)))
		return 1;

	snprintf(ret, AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS,
		 "%" AM_TIMESTAMP_T_FMT, *ts);

	*out = ret;
	*cst = 0;

	return 0;
}
