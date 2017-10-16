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

#include <QWidget>
#include <cairo.h>

#ifndef AM_CAIROWIDGET_H
#define AM_CAIROWIDGET_H

/**
 * A widget with support for rendering using cairo
 */
class CairoWidget : public QWidget
{
	public:
		typedef QWidget super;

		CairoWidget(QWidget* parent = NULL);
		virtual ~CairoWidget();

		class CairoException {};

	protected:
		void resizeEvent(QResizeEvent* e) override;
		virtual void cairoPaintEvent(cairo_t* cr) = 0;

		cairo_surface_t* getCairoSurface(void);
		cairo_t* getCairoContext();

		cairo_surface_t* cairo_surface;
		cairo_t* cairo_context;

	private:
		void paintEvent(QPaintEvent* event) override;
};

#endif
