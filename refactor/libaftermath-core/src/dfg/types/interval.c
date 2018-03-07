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

#include "interval.h"
#include <stdio.h>

int am_dfg_type_interval_to_string(const struct am_dfg_type* t,
				   void* ptr,
				   char** out,
				   int* cst)
{
	char* ret;
	struct am_interval* i = ptr;

	if(!(ret = malloc(2*AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 5)))
		return 1;

	snprintf(ret, 2 * AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 4,
		 "[%" AM_TIMESTAMP_T_FMT ", %" AM_TIMESTAMP_T_FMT "]",
		 i->start, i->end);

	*out = ret;
	*cst = 0;

	return 0;
}
