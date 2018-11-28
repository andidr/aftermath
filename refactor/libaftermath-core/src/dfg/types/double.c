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

#include <aftermath/core/dfg/types/double.h>
#include <aftermath/core/ansi_extras.h>
#include <float.h>

int am_dfg_type_double_to_string(const struct am_dfg_type* t,
			       void* ptr,
			       char** out,
			       int* cst)
{
	/* Maximum number of characters for base 10 representation of a
	 * double */
	size_t max_chars = DBL_MANT_DIG - DBL_MIN_EXP + 3;
	double* d = ptr;
	char* tmp;
	char* tmp_short;

	if(!(tmp = malloc(max_chars+1)))
		return 1;

	snprintf(tmp, max_chars+1, "%f", *d);

	/* Output with max_chars characters is very unlikely; allocate second
	 * string with only the necessary space. */
	if(!(tmp_short = strdup(tmp))) {
		free(tmp);
		return 1;
	}

	free(tmp);

	*out = tmp_short;

	return 0;
}

int am_dfg_type_double_from_string(const struct am_dfg_type* t,
				   const char* str,
				   void* out)
{
	double* d = out;

	if(am_safe_atodbln(str, strlen(str), d))
		return 1;

	return 0;
}

int am_dfg_type_double_check_string(const struct am_dfg_type* t,
				    const char* str)
{
	double d;

	if(am_safe_atodbln(str, strlen(str), &d))
		return 0;

	return 1;
}
