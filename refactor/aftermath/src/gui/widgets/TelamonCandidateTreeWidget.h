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

#ifndef AM_TELAMON_CANDIDATE_TREE_WIDGET_H
#define AM_TELAMON_CANDIDATE_TREE_WIDGET_H

#include "CairoWidgetWithDFGNode.h"
#include <aftermath/core/in_memory.h>
#include <set>
#include <vector>

extern "C" {
	#include <aftermath/render/telamon/candidate_tree_renderer.h>
}

/* A widget displaying a tree of telamon candidates */
class TelamonCandidateTreeWidget : public CairoWidgetWithDFGNode {
	public:
		TelamonCandidateTreeWidget();
		virtual ~TelamonCandidateTreeWidget();
		void setCandidateTree(struct am_telamon_candidate* root);

		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* event);
		virtual void resizeEvent(QResizeEvent *event);

		std::vector<struct am_telamon_candidate*> getSelection();

		typedef CairoWidgetWithDFGNode super;

	protected:
		/* Current mode */
		enum {
			MOUSE_MODE_NONE = 0,
			MOUSE_MODE_NAVIGATE
		} mouseMode;

		/* Position of the mouse in pixel coordinates at the beginning
		 * of a dragging operation */
		struct am_point mouseStartPos;

		/* Position of the mouse in graph coordinates at the beginning
		 * of a dragging operation */
		struct {
			double x;
			double y;
		} graphStartPos;

		/* Set with all currently selected candidates */
		std::set<struct am_telamon_candidate*> selections;

		/* Actual telamon candidate tree renderer to which all rendering
		 * is delegated */
		struct am_telamon_candidate_tree_renderer renderer;

		void mouseMoveNavigate(const struct am_point* screen_pos);

		virtual void cairoPaintEvent(cairo_t* cr);

		void checkTriggerSelectionPort();

		void toggleCandidateSelectionAt(const struct am_point* p);
		void clearSelection();
		void startNavigation(const struct am_point* p);
};

#endif
