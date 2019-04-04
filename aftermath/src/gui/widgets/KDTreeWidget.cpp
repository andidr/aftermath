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

#include "KDTreeWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>

KDTreeWidget::KDTreeWidget(QWidget* parent)
	: super(parent),
	  mouseMode(MOUSE_MODE_NONE)
{
	this->setMouseTracking(true);
	this->setCursor(Qt::OpenHandCursor);

	am_kdtree_renderer_init(&this->renderer, paintNodeCallback, this);
}

KDTreeWidget::~KDTreeWidget()
{
	am_kdtree_renderer_destroy(&this->renderer);
}

void KDTreeWidget::resizeEvent(QResizeEvent *event)
{
	super::resizeEvent(event);

	am_kdtree_renderer_set_width(&this->renderer, width());
	am_kdtree_renderer_set_height(&this->renderer, height());
}

/* Sets the k-d-tree to be rendered by the widget */
void KDTreeWidget::setKDTree(struct am_kdtree* t)
{
	am_kdtree_renderer_set_kdtree(&this->renderer, t);
}

void KDTreeWidget::cairoPaintEvent(cairo_t* cr)
{
	am_kdtree_renderer_render(&this->renderer, cr);
}

/* Called for each node to be painted; Delegates the actual painting to the
 * virtual function paintNode to be implemented by a subclass. */
void KDTreeWidget::paintNodeCallback(cairo_t* cr,
				     struct am_point screen_pos,
				     double zoom,
				     const struct am_kdtree_node* n,
				     void* data)
{
	KDTreeWidget* widget = reinterpret_cast<KDTreeWidget*>(data);
	widget->paintNode(cr, screen_pos, zoom, n);
}

void KDTreeWidget::wheelEvent(QWheelEvent* event)
{
	struct am_point p = {
		(double)event->x(),
		(double)event->y()
	};

	if(event->delta() > 0)
		am_kdtree_renderer_zoom_in(&this->renderer, &p);
	else
		am_kdtree_renderer_zoom_out(&this->renderer, &p);

	this->update();
}

/**
 * Called when the mouse is moved in navigation mode
 */
void KDTreeWidget::mouseMoveNavigate(const struct am_point* screen_pos)
{
	double dx;
	double dy;
	double gdx;
	double gdy;
	double gx;
	double gy;

	dx = this->mouseStartPos.x - screen_pos->x;
	dy = this->mouseStartPos.y - screen_pos->y;

	gdx = am_kdtree_renderer_screen_w_to_graph(&this->renderer, dx);
	gdy = am_kdtree_renderer_screen_h_to_graph(&this->renderer, dy);

	gx = this->graphStartPos.x + gdx;
	gy = this->graphStartPos.y + gdy;

	am_kdtree_renderer_set_offset(&this->renderer, gx, gy);

	this->update();
}

void KDTreeWidget::mouseMoveEvent(QMouseEvent* event)
{
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->renderer.kdtree)
		return;

	am_kdtree_renderer_screen_to_graph(&this->renderer,
					   &screen_pos,
					   &this->currMouseGraphPos);

	if(this->mouseMode == MOUSE_MODE_NAVIGATE)
		this->mouseMoveNavigate(&screen_pos);
	else
		this->setCursor(Qt::OpenHandCursor);
}

void KDTreeWidget::mousePressEvent(QMouseEvent* event)
{
	struct am_point graph_pos;
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->renderer.kdtree)
		return;

	/* Only react to left mouse button */
	if(event->button() != Qt::LeftButton)
		return;

	this->mouseStartPos.x = event->pos().x();
	this->mouseStartPos.y = event->pos().y();

	am_kdtree_renderer_screen_to_graph(&this->renderer,
					   &screen_pos,
					   &graph_pos);

	am_kdtree_renderer_get_offset(&this->renderer,
				      &this->graphStartPos.x,
				      &this->graphStartPos.y);
	this->setCursor(Qt::ClosedHandCursor);
	this->mouseMode = MOUSE_MODE_NAVIGATE;
}

void KDTreeWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if(!this->renderer.kdtree)
		return;

	this->mouseMode = MOUSE_MODE_NONE;
	this->setCursor(Qt::OpenHandCursor);

	this->update();
}
