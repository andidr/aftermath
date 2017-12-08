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

#include <aftermath/render/cairo_extras.h>

/* Draws a striped rectangle at coordinates (rect_left, rect_top) of rect_width
 * x rect_height units. The array dash specifies the dash pattern with num_dash
 * entries, indicating dash lengths (even indexes) and spaces (odd indexes). */
void am_striped_rectangle(cairo_t* cr,
			  double rect_left, double rect_top,
			  double rect_width, double rect_height,
			  double* dash, size_t num_dash)
{
	double offs = 0;
	size_t dash_idx = 0;
	double rect_right = rect_left + rect_width;
	double line_right;
	double line_top;
	double line_left;
	double line_bottom;

	while(offs < rect_width + rect_height) {
		line_right = rect_left + offs;
		line_top = rect_top;
		line_left = rect_left + offs - rect_height;
		line_bottom = rect_top + rect_height;

		if(line_right > rect_right) {
			line_top += line_right - rect_right;
			line_right = rect_right;
		}

		if(line_left < rect_left) {
			line_bottom -= rect_left - line_left;
			line_left = rect_left;
		}

		cairo_move_to(cr, line_right, line_top);
		cairo_set_line_width(cr, dash[dash_idx]);
		cairo_line_to(cr, line_left, line_bottom);

		offs += dash[dash_idx];

		if(dash_idx != num_dash - 1) {
			dash_idx++;
			offs += dash[dash_idx];
		}

		dash_idx = (dash_idx + 1) % num_dash;
	}

	cairo_rectangle(cr, rect_left, rect_top, rect_width, rect_height);
}
