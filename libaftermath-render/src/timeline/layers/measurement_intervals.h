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

#ifndef AM_TIMELINE_MEASUREMENT_INTERVALS_LAYER_H
#define AM_TIMELINE_MEASUREMENT_INTERVALS_LAYER_H

#include <aftermath/render/timeline/layer.h>

/* The measurement intervals layer renders a green vertical line with a triangle
 * pointing to the right for the beginning of each measurement interval and a
 * red vertical line with a triangle pointing to the left for the end of each
 * measurement interval. */

struct am_timeline_render_layer_type*
am_timeline_measurement_intervals_layer_instantiate_type(void);

#endif
