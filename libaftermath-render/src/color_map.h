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

#ifndef AM_COLOR_MAP_H
#define AM_COLOR_MAP_H

#include <aftermath/render/cairo_extras.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/ansi_extras.h>

/* Generates a static initializer for a color map from a static initializer of
 * an array of struct_am_rgba element. This can be used for static
 * initialization of a color map, such as:
 *
 *  struct am_color_map state_colors = AM_STATIC_COLOR_MAP({
 *			{1.0, 0.0, 0.0, 1.0},
 *			{0.0, 1.0, 0.0, 1.0},
 *			{0.0, 0.0, 1.0, 1.0}
 *		});
 */
#define AM_STATIC_COLOR_MAP(...)						\
	{									\
		.mode = AM_COLOR_MAP_MODE_MODULO,				\
		.elements = (struct am_rgba[])__VA_ARGS__,			\
		.num_elements = AM_ARRAY_SIZE(					\
			(struct am_rgba[])AM_MACRO_ARG_PROTECT(__VA_ARGS__))	\
	}

/* Defines which color should be returned for an index greater than the number
 * of colors in a color map */
enum am_color_map_mode {
	/* Wrap around maximum color */
	AM_COLOR_MAP_MODE_MODULO = 0,

	/* Returns the last color */
	AM_COLOR_MAP_MODE_MAX,
};

#define AM_COLOR_MAP_EXTRA_FIELDS \
	enum am_color_map_mode mode;

AM_DECL_TYPED_ARRAY_EXTRA_FIELDS(am_color_map,
				 struct am_rgba,
				 AM_COLOR_MAP_EXTRA_FIELDS);

static inline void am_color_map_set_mode(struct am_color_map* cm,
					 enum am_color_map_mode mode)
{
	cm->mode = mode;
}

/* Returns the num-th color of the color map in *color. If no such color exists
 * (e.g., if the coor map is empty), the function returns NULL. */
static inline const struct am_rgba*
am_color_map_get_color(const struct am_color_map* cm,
		       size_t num)
{
	size_t idx;

	if(cm->num_elements == 0)
		return NULL;

	if(cm->mode == AM_COLOR_MAP_MODE_MODULO)
		idx = num % cm->num_elements;
	else
		idx = (num >= cm->num_elements) ? cm->num_elements-1 : num;

	return &cm->elements[idx];
}

#endif
