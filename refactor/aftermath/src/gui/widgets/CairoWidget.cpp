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

#include "CairoWidget.h"
#include <QPainter>

CairoWidget::CairoWidget(QWidget *parent)
	: super(parent),
	  cairo_surface(NULL), cairo_context(NULL)
{
}

void CairoWidget::paintEvent(QPaintEvent* e)
{
	unsigned char* data;

	this->cairoPaintEvent(this->cairo_context);

	if(!(data = cairo_image_surface_get_data(this->cairo_surface)))
		throw CairoException();

	QPainter painter(this);
	QImage img(data, this->width(), this->height(), QImage::Format_ARGB32);
	painter.drawImage(this->rect(), img, this->rect());
}

void CairoWidget::resizeEvent(QResizeEvent* event)
{
	if(this->cairo_context) {
		cairo_destroy(this->cairo_context);
		this->cairo_context = NULL;
	}

	if(this->cairo_surface) {
		cairo_surface_destroy(this->cairo_surface);
		this->cairo_surface = NULL;
	}

	if(!(this->cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
							this->width(),
							this->height())))
	{
		throw CairoException();
	}

	if(!(this->cairo_context = cairo_create(this->cairo_surface))) {
		throw CairoException();
	}
}

cairo_t* CairoWidget::getCairoContext()
{
	return cairo_context;
}

CairoWidget::~CairoWidget()
{
	if(this->cairo_context)
		cairo_destroy(this->cairo_context);

	if(this->cairo_surface)
		cairo_surface_destroy(this->cairo_surface);
}
