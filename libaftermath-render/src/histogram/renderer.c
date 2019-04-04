/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "renderer.h"

int am_histogram_renderer_init(struct am_histogram_renderer* r)
{
	r->params.bgcolor = AM_RGBA255(0x00, 0x00, 0x00, 0xFF);
	r->params.bin_color = AM_RGBA255(0xFF, 0xFF, 0x00, 0xFF);
	r->params.outline.color = AM_RGBA255(0xFF, 0x0, 0x00, 0xFF);
	r->params.outline.width = 0.5;

	r->width = 0;
	r->height = 0;
	r->histogram_data = NULL;

	return 0;
}

void am_histogram_renderer_destroy(struct am_histogram_renderer* r)
{
}

/* Sets the histogram for the renderer */
void am_histogram_renderer_set_histogram(struct am_histogram_renderer* r,
					 const struct am_histogram1d_data* d)
{
	r->histogram_data = d;
}

void am_histogram_renderer_paint_background(struct am_histogram_renderer* r,
					    cairo_t* cr)
{
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(r->params.bgcolor));
	cairo_rectangle(cr, 0, 0, r->width, r->height);
	cairo_fill(cr);
}

void am_histogram_renderer_paint_data(struct am_histogram_renderer* r,
				      cairo_t* cr)
{
	size_t n = r->histogram_data->num_bins;
	double nd = n;
	double heightd = r->height;
	double widthd = r->width;
	uint64_t max = 0;
	double bin_height;
	double bin_vald;
	double maxd;
	double y;
	double x;

	/* Find bin with the maximum value */
	for(size_t i = 0; i < n; i++) {
		if(r->histogram_data->bins[i] > max)
			max = r->histogram_data->bins[i];
	}

	/* Only zero values -> nothing to paint */
	if(max == 0)
		return;

	maxd = max;

	cairo_move_to(cr, 0, r->height);
	y = r->height;

	for(size_t i = 0; i < n; i++) {
		x = (((double)i) / nd) * widthd;

		cairo_line_to(cr, x, y);

		bin_vald = r->histogram_data->bins[i];
		bin_height = (bin_vald / maxd) * heightd;
		y = r->height - bin_height;

		cairo_line_to(cr, x, y);
	}

	cairo_line_to(cr, r->width, y);
	cairo_line_to(cr, r->width, r->height);

	cairo_set_source_rgba(cr, AM_RGBA_ARGS(r->params.bin_color));
	cairo_fill_preserve(cr);

	cairo_set_line_width(cr, r->params.outline.width);
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(r->params.outline.color));
	cairo_stroke(cr);
}

void am_histogram_renderer_render(struct am_histogram_renderer* r, cairo_t* cr)
{
	am_histogram_renderer_paint_background(r, cr);

	if(!r->histogram_data)
		return;

	am_histogram_renderer_paint_data(r, cr);
}
