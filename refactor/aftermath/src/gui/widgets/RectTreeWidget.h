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

#ifndef AM_RECTTREEWIDGET_H
#define AM_RECTTREEWIDGET_H

#include "CairoWidget.h"
#include "../../Exception.h"

extern "C" {
	#include <aftermath/render/recttree/renderer.h>
}

/**
 * Widget encapsulating a renderer for rect trees
 */
class RectTreeWidget : public CairoWidget {
	Q_OBJECT

	public:
		typedef CairoWidget super;

		RectTreeWidget(QWidget* parent = NULL);
		virtual ~RectTreeWidget();
		virtual void resizeEvent(QResizeEvent *event);

		void setRectTree(struct am_recttree* t);

		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* event);

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

		struct am_point currMouseGraphPos;

		struct am_recttree_renderer renderer;

		virtual void cairoPaintEvent(cairo_t* cr);

		/* Pure virtual function that carries out the actual painting of
		 * a rectangle; Must be implemented by the subclass. */
		virtual void paintRect(cairo_t* cr,
				       struct am_rect screen_pos,
				       double zoom,
				       const struct am_recttree_node* n) = 0;

		static void paintRectCallback(cairo_t* cr,
					      struct am_rect screen_pos,
					      double zoom,
					      const struct am_recttree_node* n,
					      void* data);

		void mouseMoveNavigate(const struct am_point* screen_pos);

		virtual void renderParametersChanged();
};

#endif
