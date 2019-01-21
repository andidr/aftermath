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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "CairoWidgetWithDFGNode.h"

extern "C" {
	#include <aftermath/render/timeline/renderer.h>
}

struct am_timeline_entity;

/**
 * Widget encapsulating the time line renderer showing events over time.
 */
class TimelineWidget : public CairoWidgetWithDFGNode {
	public:
		typedef CairoWidgetWithDFGNode super;
		class TimelineWidgetException {};

		TimelineWidget(QWidget* parent = NULL);
		virtual ~TimelineWidget();
		virtual void resizeEvent(QResizeEvent *event);
		void setTrace(struct am_trace* t);
		void setHierarchy(struct am_hierarchy* h);
		void setVisibleInterval(const struct am_interval* i);
		void addLayer(struct am_timeline_render_layer* l);

		void getVisibleInterval(struct am_interval* i);
		size_t getNumSelections();
		void storeSelectionIntervals(struct am_interval* out, size_t max);

		am_timestamp_t getLastTimestampUnderCursor();
		struct am_hierarchy_node* getLastHierarchyNodeUnderCursor();

		struct am_timeline_renderer* getRenderer();

	protected:
		enum zoomDirection {
			ZOOM_IN,
			ZOOM_OUT
		};

		virtual void cairoPaintEvent(cairo_t* cr);

		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* event);

		virtual bool handleMouseMoveHierarchyLayerItem(
			QMouseEvent* event,
			const struct am_timeline_entity* e);

		virtual bool handleMousePressHierarchyLayerItem(
			QMouseEvent* event,
			const struct am_timeline_entity* e);

		virtual bool handleMouseMoveAxesLayerItem(
			QMouseEvent* event,
			const struct am_timeline_entity* e);

		virtual bool handleMousePressAxesLayerItem(
			QMouseEvent* event,
			const struct am_timeline_entity* e);

		virtual bool handleMouseMoveSelectionLayerItem(
			QMouseEvent* event,
			const struct am_timeline_entity* e);

		virtual bool handleMousePressSelectionLayerItem(
			QMouseEvent* event,
			const struct am_timeline_entity* e);

		virtual void handleHorizontalAxisDragEvent(double y);
		virtual void handleVerticalAxisDragEvent(double x);
		virtual void handleLanesDragEvent(double x);
		virtual void handleSelectionDragEvent(double x);
		virtual void handleDragEvent(QMouseEvent* event);
		virtual void handleZoomEvent(double x, enum zoomDirection);

		bool checkStartCreateSelection(QMouseEvent* event);
		void checkTriggerSelectionPort();

		void checkUpdateMousePos(const struct am_point* p);

		struct am_timeline_renderer renderer;
		struct am_timeline_renderer_layer_appearance_change_callback
		apperance_change_cb;

		enum {
			MOUSE_MODE_NONE = 0,
			MOUSE_MODE_DRAG_HORIZONTAL_AXIS,
			MOUSE_MODE_DRAG_VERTICAL_AXIS,
			MOUSE_MODE_DRAG_LANES,
			MOUSE_MODE_DRAG_SELECTION_START,
			MOUSE_MODE_DRAG_SELECTION_END
		} mouseMode;

		struct {
			QPoint pos;
			struct am_interval visibleInterval;
		} dragStart;

		bool mouseMovedSincePress;

		struct {
			uint64_t value;
			uint64_t multiplier;
		} zoom;

		double ylegendScrollPx;

		std::vector<struct am_timeline_selection_layer*> selectionLayers;
		struct am_timeline_selection_layer* defaultSelectionLayer;
		struct am_timeline_selection_layer* currentSelectionLayer;
		const struct am_timeline_selection* currentSelection;

		struct am_hierarchy_node* lastHierarchyNodeUnderCursor;
		am_timestamp_t lastTimestampUnderCursor;
};

#endif
