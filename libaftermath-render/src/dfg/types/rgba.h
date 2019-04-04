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

#ifndef AM_RENDER_DFG_TYPE_RGBA_H
#define AM_RENDER_DFG_TYPE_RGBA_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/dfg/types/generic.h>
#include <aftermath/render/cairo_extras.h>

int am_render_dfg_type_rgba_to_string(const struct am_dfg_type* t,
				      void* ptr,
				      char** out,
				      int* cst);

int am_render_dfg_type_rgba_from_string(const struct am_dfg_type* t,
					const char* str,
					void* out);

int am_render_dfg_type_rgba_check_string(const struct am_dfg_type* t,
					 const char* str);

AM_DFG_DECL_BUILTIN_TYPE(
	am_render_dfg_type_rgba,
	"am::render::rgba",
	sizeof(struct am_rgba),
	NULL,
	am_dfg_type_generic_plain_copy_samples,
	am_render_dfg_type_rgba_to_string,
	am_render_dfg_type_rgba_from_string,
	am_render_dfg_type_rgba_check_string)

AM_DFG_ADD_BUILTIN_TYPES(&am_render_dfg_type_rgba)

#endif
