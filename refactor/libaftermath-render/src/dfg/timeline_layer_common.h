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

#ifndef AM_RENDER_DFG_TIMELINE_LAYER_COMMON_H
#define AM_RENDER_DFG_TIMELINE_LAYER_COMMON_H

#include <aftermath/core/dfg_type.h>

/* Declares a DFG type "const am::render::timeline::layer::<ident_name>*" for
 * pointers to a render layer */
#define AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(layer_name, ident_name)		\
	AM_DFG_DECL_BUILTIN_TYPE(						\
		am_render_dfg_type_timeline_##layer_name##_layer,		\
		"const am::render::timeline::layer::" ident_name "*",		\
		/* The exact type doesn't really matter here, since the size of \
		 * pointers is the same for all layers */			\
		sizeof(struct am_timeline_render_layer*),			\
		NULL, NULL, NULL, NULL)

#endif
