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

#include "RectTreeWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>

RectTreeWidget::RectTreeWidget(QWidget* parent)
	: super(parent),
	  mouseMode(MOUSE_MODE_NONE)
{
	this->setMouseTracking(true);
	this->setCursor(Qt::OpenHandCursor);

	am_recttree_renderer_init(&this->renderer, paintRectCallback, this);
}

RectTreeWidget::~RectTreeWidget()
{
	am_recttree_renderer_destroy(&this->renderer);
}

void RectTreeWidget::resizeEvent(QResizeEvent *event)
{
	super::resizeEvent(event);

	am_recttree_renderer_set_width(&this->renderer, width());
	am_recttree_renderer_set_height(&this->renderer, height());

	this->renderParametersChanged();
}

/* Sets the rect tree to be rendered by the widget */
void RectTreeWidget::setRectTree(struct am_recttree* t)
{
	am_recttree_renderer_set_recttree(&this->renderer, t);
}

void RectTreeWidget::cairoPaintEvent(cairo_t* cr)
{
	am_recttree_renderer_render(&this->renderer, cr);
}

/* Called for each rectangle to be painted; Delegates the actual painting to the
 * virtual function paintRect to be implemented by a subclass. */
void RectTreeWidget::paintRectCallback(cairo_t* cr,
				     struct am_rect screen_rect,
				     double zoom,
				     const struct am_recttree_node* n,
				     void* data)
{
	RectTreeWidget* widget = reinterpret_cast<RectTreeWidget*>(data);
	widget->paintRect(cr, screen_rect, zoom, n);
}

void RectTreeWidget::wheelEvent(QWheelEvent* event)
{
	struct am_point p = {
		(double)event->x(),
		(double)event->y()
	};

	if(event->delta() > 0)
		am_recttree_renderer_zoom_in(&this->renderer, &p);
	else
		am_recttree_renderer_zoom_out(&this->renderer, &p);

	this->renderParametersChanged();
	this->update();
}

/**
 * Called when the mouse is moved in navigation mode
 */
void RectTreeWidget::mouseMoveNavigate(const struct am_point* screen_pos)
{
	double dx;
	double dy;
	double gdx;
	double gdy;
	double gx;
	double gy;

	dx = this->mouseStartPos.x - screen_pos->x;
	dy = this->mouseStartPos.y - screen_pos->y;

	gdx = am_recttree_renderer_screen_w_to_graph(&this->renderer, dx);
	gdy = am_recttree_renderer_screen_h_to_graph(&this->renderer, dy);

	gx = this->graphStartPos.x + gdx;
	gy = this->graphStartPos.y + gdy;

	am_recttree_renderer_set_offset(&this->renderer, gx, gy);

	this->renderParametersChanged();
	this->update();
}

void RectTreeWidget::mouseMoveEvent(QMouseEvent* event)
{
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->renderer.recttree)
		return;

	am_recttree_renderer_screen_to_graph(&this->renderer,
					   &screen_pos,
					   &this->currMouseGraphPos);

	if(this->mouseMode == MOUSE_MODE_NAVIGATE)
		this->mouseMoveNavigate(&screen_pos);
	else
		this->setCursor(Qt::OpenHandCursor);
}

void RectTreeWidget::mousePressEvent(QMouseEvent* event)
{
	struct am_point graph_pos;
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->renderer.recttree)
		return;

	/* Only react to left mouse button */
	if(event->button() != Qt::LeftButton)
		return;

	this->mouseStartPos.x = event->pos().x();
	this->mouseStartPos.y = event->pos().y();

	am_recttree_renderer_screen_to_graph(&this->renderer,
					   &screen_pos,
					   &graph_pos);

	am_recttree_renderer_get_offset(&this->renderer,
				      &this->graphStartPos.x,
				      &this->graphStartPos.y);
	this->setCursor(Qt::ClosedHandCursor);
	this->mouseMode = MOUSE_MODE_NAVIGATE;
}

void RectTreeWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if(!this->renderer.recttree)
		return;

	this->mouseMode = MOUSE_MODE_NONE;
	this->setCursor(Qt::OpenHandCursor);

	this->update();
}

void RectTreeWidget::renderParametersChanged()
{
}
