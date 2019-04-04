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

#include <aftermath/render/dfg/types/rgba.h>

int am_render_dfg_type_rgba_to_string(const struct am_dfg_type* t,
				      void* ptr,
				      char** out,
				      int* cst)
{
	struct am_rgba* color = ptr;

	if(!(*out = am_rgba_to_string_alloc(color)))
		return 1;

	*cst = 0;

	return 0;
}

int am_render_dfg_type_rgba_from_string(const struct am_dfg_type* t,
					const char* str,
					void* out)
{
	struct am_rgba* color = out;

	if(am_rgba_from_string(color, str))
		return 1;

	return 0;
}

int am_render_dfg_type_rgba_check_string(const struct am_dfg_type* t,
					 const char* str)
{
	struct am_rgba color;

	if(am_rgba_from_string(&color, str))
		return 0;

	return 1;
}
