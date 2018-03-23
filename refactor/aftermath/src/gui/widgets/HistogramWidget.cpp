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

#include "HistogramWidget.h"

HistogramWidget::HistogramWidget(QWidget* parent) :
	super(parent), data(NULL)
{
	if(am_histogram_renderer_init(&this->renderer))
		throw Exception("Could not initialize histogram renderer");

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

HistogramWidget::~HistogramWidget()
{
	this->setHistogram(NULL);
	am_histogram_renderer_destroy(&this->renderer);
}

void HistogramWidget::cairoPaintEvent(cairo_t* cr)
{
	am_histogram_renderer_render(&this->renderer, cr);
}

void HistogramWidget::resizeEvent(QResizeEvent *event)
{
	super::resizeEvent(event);

	am_histogram_renderer_set_width(&this->renderer, this->width());
	am_histogram_renderer_set_height(&this->renderer, this->height());
}

/* Sets the histogram to be displayed by the widget. Ownership of d is
 * transferred to the widget.
 */
void HistogramWidget::setHistogram(struct am_histogram1d_data* d)
{
	if(this->data) {
		am_histogram1d_data_destroy(this->data);
		free(this->data);
	}

	this->data = d;
	am_histogram_renderer_set_histogram(&this->renderer, d);

	this->update();
}

/* Returns the histogram currently displayed by the widget or NULL if no
 * histogram has been set. */
const struct am_histogram1d_data* HistogramWidget::getHistogram()
{
	return am_histogram_renderer_get_histogram(&this->renderer);
}
