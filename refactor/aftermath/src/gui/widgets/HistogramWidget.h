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

#ifndef AM_HISTOGRAM_WIDGET_H
#define AM_HISTOGRAM_WIDGET_H

#include "CairoWidgetWithDFGNode.h"
#include "../../Exception.h"

extern "C" {
	#include <aftermath/render/histogram/renderer.h>
}

/**
 * Widget encapsulating the histogram renderer.
 */
class HistogramWidget : public CairoWidgetWithDFGNode {
	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg)
				{ }
		};

		typedef CairoWidgetWithDFGNode super;

		HistogramWidget(QWidget* parent = NULL);
		~HistogramWidget();
		void setHistogram(struct am_histogram1d_data* d);
		const struct am_histogram1d_data* getHistogram();

		virtual void resizeEvent(QResizeEvent *event);

	protected:
		struct am_histogram_renderer renderer;
		virtual void cairoPaintEvent(cairo_t* cr);

		struct am_histogram1d_data* data;

};

#endif
