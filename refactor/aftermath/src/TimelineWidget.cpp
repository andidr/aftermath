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

#include "TimelineWidget.h"
#include <QMouseEvent>
#include <iostream>
#include <aftermath/render/timeline/layers/hierarchy.h>

TimelineWidget::TimelineWidget(QWidget* parent)
	: super(parent)
{
	if(am_timeline_renderer_init(&this->renderer))
		throw TimelineWidgetException();

	this->setMouseTracking(true);
}

void TimelineWidget::cairoPaintEvent(cairo_t* cr)
{
	am_timeline_renderer_render(&this->renderer, cr);
}

void TimelineWidget::resizeEvent(QResizeEvent* event)
{
	super::resizeEvent(event);

	am_timeline_renderer_set_width(&this->renderer, width());
	am_timeline_renderer_set_height(&this->renderer, height());
}

/**
 * Set the trace whose events are to be displayed by the time line
 */
void TimelineWidget::setTrace(struct am_trace* t)
{
	am_timeline_renderer_set_trace(&this->renderer, t);
}

/**
 * Set the hierarchy whose events are to be displayed by the time line
 */
void TimelineWidget::setHierarchy(struct am_hierarchy* h)
{
	if(am_timeline_renderer_set_hierarchy(&this->renderer, h))
		throw TimelineWidgetException();
}

/* Sets the visible interval. If the interval is invalid, an exception is
 * thrown. */
void TimelineWidget::setVisibleInterval(const struct am_interval* i)
{
	if(i->end < i->start)
		throw TimelineWidgetException();

	am_timeline_renderer_set_visible_interval(&this->renderer, i);

	this->update();
}

/**
 * Add a time line render layer to the time line renderer
 */
void TimelineWidget::addLayer(struct am_timeline_render_layer* l)
{
	am_timeline_renderer_add_layer(&this->renderer, l);
}

TimelineWidget::~TimelineWidget()
{
	am_timeline_renderer_destroy(&this->renderer);
}

/**
 * Handle mouse move event on a position with a hierarchy layer item.
 */
void TimelineWidget::handleMouseMoveHierarchyLayerItem(
	QMouseEvent* event,
	const struct am_timeline_entity* e)
{
	struct am_timeline_hierarchy_layer_collapse_button* cbtn;

	/* Hovering over a collapse button */
	if(e->type == AM_TIMELINE_HIERARCHY_LAYER_ENTITY_COLLAPSE_BUTTON) {
		cbtn = (typeof(cbtn))e;

		if(am_hierarchy_node_has_children(cbtn->node))
			this->setCursor(Qt::PointingHandCursor);
	}
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
	struct list_head l;
	struct am_timeline_entity* e;

	this->setCursor(Qt::ArrowCursor);

	am_timeline_renderer_identify_entities(&this->renderer, &l, event->x(), event->y());

	am_typed_list_for_each_genentry(&l, e, list) {
		if(strcmp(e->layer->type->name, "hierarchy") == 0)
			this->handleMouseMoveHierarchyLayerItem(event, e);
	}

	am_timeline_renderer_destroy_entities(&this->renderer, &l);
}

/**
 * Handle mouse press event on a position with a hierarchy layer item.
 */
void TimelineWidget::handleMousePressHierarchyLayerItem(
	QMouseEvent* event,
	const struct am_timeline_entity* e)
{
	struct am_timeline_hierarchy_layer_collapse_button* cbtn;

	/* Clicking on a collapse button */
	if(e->type == AM_TIMELINE_HIERARCHY_LAYER_ENTITY_COLLAPSE_BUTTON) {
		cbtn = (typeof(cbtn))e;

		if(am_hierarchy_node_has_children(cbtn->node)) {
			am_timeline_renderer_toggle_node_idx(&this->renderer,
							     cbtn->node_idx);
			this->update();
		}
	}
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
	struct list_head l;
	struct am_timeline_entity* e;

	this->setCursor(Qt::ArrowCursor);

	am_timeline_renderer_identify_entities(&this->renderer, &l, event->x(), event->y());

	am_typed_list_for_each_genentry(&l, e, list) {
		if(strcmp(e->layer->type->name, "hierarchy") == 0)
			this->handleMousePressHierarchyLayerItem(event, e);
	}

	am_timeline_renderer_destroy_entities(&this->renderer, &l);
}
