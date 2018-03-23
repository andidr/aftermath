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

#ifndef AM_HISTOGRAM_RENDERER_H
#define AM_HISTOGRAM_RENDERER_H

#include <cairo.h>
#include <aftermath/core/statistics/histogram.h>
#include <aftermath/render/cairo_extras.h>

struct am_histogram_renderer_params {
	/* Background color of the histogram graph */
	struct am_rgba bgcolor;

	/* Color for the bins of the histogram */
	struct am_rgba bin_color;

	struct {
		/* Color for the line around the graph */
		struct am_rgba color;

		/* Width in pixels for the outline */
		double width;
	} outline;
};

struct am_histogram_renderer {
	/* Rendering parameters */
	struct am_histogram_renderer_params params;

	/* Width in pixels of the visible portion of the histogram */
	unsigned int width;

	/* Height in pixels of the visible portion of the histogram */
	unsigned int height;

	const struct am_histogram1d_data* histogram_data;
};

int am_histogram_renderer_init(struct am_histogram_renderer* r);
void am_histogram_renderer_destroy(struct am_histogram_renderer* r);
void am_histogram_renderer_render(struct am_histogram_renderer* r, cairo_t* cr);

void am_histogram_renderer_set_histogram(struct am_histogram_renderer* r,
					 const struct am_histogram1d_data* d);

/* Returns the histogram data currently associated with the renderer or NULL if
 * the histogram has not been set. */
inline const struct am_histogram1d_data*
am_histogram_renderer_get_histogram(struct am_histogram_renderer* r)
{
	return r->histogram_data;
}

/* Sets the width in pixels of the renderer. */
static inline void
am_histogram_renderer_set_width(struct am_histogram_renderer* r,
				unsigned int w)
{
	r->width = w;
}

/* Sets the height in pixels of the renderer. */
static inline void
am_histogram_renderer_set_height(struct am_histogram_renderer* r,
				 unsigned int h)
{
	r->height = h;
}

#endif
