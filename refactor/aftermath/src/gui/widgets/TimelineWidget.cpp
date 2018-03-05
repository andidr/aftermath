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

extern "C" {
	#include <aftermath/render/timeline/layers/hierarchy.h>
	#include <aftermath/render/timeline/layers/axes.h>
	#include <aftermath/core/timestamp.h>
}

TimelineWidget::TimelineWidget(QWidget* parent)
	: super(parent), mouseMode(MOUSE_MODE_NONE),
	  zoom({1100, 1000}),
	  ylegendScrollPx(20)
{
	this->dragStart.visibleInterval = { 0, 0 };

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

/* Retrieves the currently visible interval
 */
void TimelineWidget::getVisibleInterval(struct am_interval* i)
{
	am_timeline_renderer_get_visible_interval(&this->renderer, i);
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

/**
 * Handle mouse move event on a position with a axes layer item.
 */
void TimelineWidget::handleMouseMoveAxesLayerItem(
	QMouseEvent* event,
	const struct am_timeline_entity* e)
{
	struct am_timeline_axes_layer_axis* axis;

	/* Hovering over an axis */
	if(e->type == AM_TIMELINE_AXES_LAYER_ENTITY_AXIS) {
		axis = (typeof(axis))e;

		switch(axis->type) {
			case AM_TIMELINE_AXES_LAYER_AXIS_TYPE_HORIZONTAL:
				this->setCursor(Qt::SizeVerCursor);
				break;
			case AM_TIMELINE_AXES_LAYER_AXIS_TYPE_VERTICAL:
				this->setCursor(Qt::SizeHorCursor);
				break;
		}
	}
}

/**
 * Adjusts the vertical position of the horizontal axis and the space below.
 */
void TimelineWidget::handleHorizontalAxisDragEvent(double y)
{
	am_timeline_renderer_set_horizontal_axis_y(&this->renderer, y);
	this->update();
}

/**
 * Adjusts the horizontal position of the vertical axis and the space on its
 * left.
 */
void TimelineWidget::handleVerticalAxisDragEvent(double x)
{
	am_timeline_renderer_set_vertical_axis_x(&this->renderer, x);
	this->update();
}

/**
 * Handle a mouve move event with pressed mouse button on the timeline
 * lanes. Shifts the visible interval accordingly.
 */
void TimelineWidget::handleLanesDragEvent(double x)
{
	struct am_timeline_renderer* r = &this->renderer;
	double xdiff = this->dragStart.pos.x() - x;
	struct am_time_offset tdiff = {0, 0};
	struct am_interval i;

	if(am_timeline_renderer_width_to_duration(r, xdiff, &tdiff))
		return;

	i = this->dragStart.visibleInterval;

	am_interval_shift(&i, &tdiff);
	am_timeline_renderer_set_visible_interval(r, &i);

	this->update();
}

/**
 * Handles a mouse move event with the mouse button pressed
 */
void TimelineWidget::handleDragEvent(QMouseEvent* event)
{
	switch(this->mouseMode) {
		case MOUSE_MODE_DRAG_HORIZONTAL_AXIS:
			this->handleHorizontalAxisDragEvent(event->y());
			break;
		case MOUSE_MODE_DRAG_VERTICAL_AXIS:
			this->handleVerticalAxisDragEvent(event->x());
			break;
		case MOUSE_MODE_DRAG_LANES:
			this->handleLanesDragEvent(event->x());
			break;
		default:
			break;
	}
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
	struct list_head l;
	struct am_timeline_entity* e;
	double x = (double)event->x();
	double y = (double)event->y();
	struct am_point p = { .x = x, .y = y };

	if(this->mouseMode == MOUSE_MODE_NONE) {
		/* Hovering over lanes? */
		if((am_point_in_rect(&p, &this->renderer.rects.lanes)))
			this->setCursor(Qt::OpenHandCursor);
		else
			this->setCursor(Qt::ArrowCursor);

		/* Get entities below the cursor */
		am_timeline_renderer_identify_entities(&this->renderer, &l, x, y);

		am_typed_list_for_each_genentry(&l, e, list) {
			if(strcmp(e->layer->type->name, "hierarchy") == 0) {
				this->handleMouseMoveHierarchyLayerItem(event, e);
				break;
			} else if(strcmp(e->layer->type->name, "axes") == 0) {
				this->handleMouseMoveAxesLayerItem(event, e);
				break;
			}
		}

		am_timeline_renderer_destroy_entities(&this->renderer, &l);
	} else {
		this->handleDragEvent(event);
	}
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

/**
 * Handle mouse press event on a position with a axes layer item.
 */
void TimelineWidget::handleMousePressAxesLayerItem(
	QMouseEvent* event,
	const struct am_timeline_entity* e)
{
	struct am_timeline_axes_layer_axis* axis;

	/* Start drag on an axis */
	if(e->type == AM_TIMELINE_AXES_LAYER_ENTITY_AXIS) {
		axis = (typeof(axis))e;

		switch(axis->type) {
			case AM_TIMELINE_AXES_LAYER_AXIS_TYPE_HORIZONTAL:
				this->mouseMode = MOUSE_MODE_DRAG_HORIZONTAL_AXIS;
				break;
			case AM_TIMELINE_AXES_LAYER_AXIS_TYPE_VERTICAL:
				this->mouseMode = MOUSE_MODE_DRAG_VERTICAL_AXIS;
				break;
		}

		this->dragStart.pos = event->pos();
	}
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
	struct list_head l;
	struct am_timeline_entity* e;
	int empty;

	struct am_point p = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	/* Find entities below the cursor */
	am_timeline_renderer_identify_entities(&this->renderer, &l, p.x, p.y);

	empty = list_empty(&l);

	am_typed_list_for_each_genentry(&l, e, list) {
		if(strcmp(e->layer->type->name, "hierarchy") == 0) {
			this->handleMousePressHierarchyLayerItem(event, e);
			break;
		} else if(strcmp(e->layer->type->name, "axes") == 0) {
			this->handleMousePressAxesLayerItem(event, e);
			break;
		}
	}

	am_timeline_renderer_destroy_entities(&this->renderer, &l);

	if(empty) {
		/* Start navigation on the lanes */
		if(am_point_in_rect(&p, &this->renderer.rects.lanes)) {
			this->mouseMode = MOUSE_MODE_DRAG_LANES;
			this->dragStart.visibleInterval =
				this->renderer.visible_interval;
			this->dragStart.pos = event->pos();
			this->setCursor(Qt::ClosedHandCursor);
		}
	}
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
{
	struct am_timeline_renderer* r = &this->renderer;

	struct am_point p = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(am_point_in_rect(&p, &r->rects.lanes))
		this->setCursor(Qt::OpenHandCursor);
	else
		this->setCursor(Qt::ArrowCursor);

	this->mouseMode = MOUSE_MODE_NONE;
}

/* Handle a zoom event at X position x*/
void TimelineWidget::handleZoomEvent(double x, enum zoomDirection direction)
{
	struct am_timeline_renderer* r = &this->renderer;

	/* Timestamp at the mouse position */
	am_timestamp_t t;

	struct am_interval i_new = r->visible_interval;
	struct am_interval i_old = r->visible_interval;

	/* Duration of the visible interval before the zoom */
	struct am_time_offset d;

	/* Duration by which the interval is increased at both sides */
	am_timestamp_t inc;

	/* Offset by how much the widened interval is shifted in order to
	 * preserve the timestamp under the cursor at the same pixel position */
	am_timestamp_t offs;

	am_timestamp_t tmp;

	am_timeline_renderer_x_to_timestamp(r, x, &t);

	am_interval_duration(&i_old, &d);

	inc = d.abs;
	am_timestamp_muldiv_sat(&inc,
				this->zoom.value - this->zoom.multiplier,
				this->zoom.multiplier);

	/* Precision not high enough; manually adjust in order to prevent
	 * blocking zoom */
	if(inc == 0)
		inc = 1;

	if(direction == ZOOM_IN)
		am_interval_shrink_u(&i_new, inc);
	else
		am_interval_widen_u(&i_new, inc);

	if(t < am_interval_middle(&i_old)) {
		/* Saturated calculation of:
		 * offs = inc - 2 * inc * (t - old_i.start) / d
		 */
		tmp = 2;
		am_timestamp_mul_sat(&tmp, inc);
		am_timestamp_muldiv_sat(&tmp, t - i_old.start, d.abs);
		offs = inc;
		am_timestamp_sub_sat(&offs, tmp);

		if(direction == ZOOM_IN)
			am_interval_shift_left_u(&i_new, offs);
		else
			am_interval_shift_right_u(&i_new, offs);
	} else {
		/* Saturated calculation of:
		 * offs = 2 * inc * (t - old_i.start) / d - inc
		 */
		offs = 2;
		am_timestamp_mul_sat(&offs, inc);
		am_timestamp_muldiv_sat(&offs, t - i_old.start, d.abs);
		am_timestamp_sub_sat(&offs, inc);

		if(direction == ZOOM_IN)
			am_interval_shift_right_u(&i_new, offs);
		else
			am_interval_shift_left_u(&i_new, offs);
	}

	am_timeline_renderer_set_visible_interval(r, &i_new);

	this->update();
}

void TimelineWidget::wheelEvent(QWheelEvent* event)
{
	struct am_timeline_renderer* r = &this->renderer;
	struct am_point p = { .x = (double)event->x(), .y = (double)event->y() };
	double offs;

	if(am_point_in_rect(&p, &r->rects.ylegend)) {
		/* Scrolling on left part with the legens for the Y axis: scroll
		 * up / down the hierarchy */
		offs = am_timeline_renderer_get_lane_offset(r);
		offs -= (event->delta() / 100.0) * this->ylegendScrollPx;

		am_timeline_renderer_set_lane_offset(r, offs);
		this->update();
	} else if(am_point_in_rect(&p, &r->rects.lanes)) {
		/* Scrolling on the time line lanes: zoom in and out */
		if(event->delta() > 0)
			this->handleZoomEvent(p.x, ZOOM_IN);
		else
			this->handleZoomEvent(p.x, ZOOM_OUT);
	} else {
		event->ignore();
	}
}
