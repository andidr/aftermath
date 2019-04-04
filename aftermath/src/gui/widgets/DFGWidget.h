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

#ifndef DFGWIDGET_H
#define DFGWIDGET_H

#include "CairoWidget.h"
#include "../../Exception.h"

extern "C" {
	#include <aftermath/render/dfg/renderer.h>
}

/**
 * Widget encapsulating the DFG line renderer showing data flow graphs.
 */
class DFGWidget : public CairoWidget {
	Q_OBJECT

	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg)
				{ }
		};

		typedef CairoWidget super;
		class DFGWidgetException {};

		DFGWidget(QWidget* parent = NULL);
		virtual ~DFGWidget();
		virtual void resizeEvent(QResizeEvent *event);

		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		virtual void mouseDoubleClickEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* event);

		void setGraph(struct am_dfg_graph* g);
		void setCoordinateMapping(struct am_dfg_coordinate_mapping* m);
		void setTypeRegistry(struct am_dfg_type_registry* tr);
		void saveGraph(const QString& filename);
		void saveGraph();

	protected:
		struct am_dfg_renderer renderer;
		struct am_dfg_path cycle;

		struct am_dfg_graph* graph;
		struct am_dfg_coordinate_mapping* coordinate_mapping;

		/* Parameters for error messages displayed on top of the DFG */
		struct {
			struct {
				const char* font;
				struct am_rgba color;
				struct am_rgba bgcolor;
				struct am_rgba line_color;
				double font_size;
				double padding_x;
				double padding_y;
				double margin_x;
				double margin_y;
			} error;
		} params;

		/* Current mode */
		enum {
			MOUSE_MODE_NONE = 0,
			MOUSE_MODE_NAVIGATE,
			MOUSE_MODE_DRAG_NODE,
			MOUSE_MODE_DRAG_PORT
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

		/* Possible errors when trying to connect two ports */
		enum PortCompatibility {
			/* Ports are compatible (no error) */
			COMPATIBLE = 0,

			/* Trying to connect two output ports */
			OUTPUT_TO_OUTPUT,

			/* Trying to connect two input ports */
			INPUT_TO_INPUT,

			/* Trying to connect two ports with incompatible element
			 * types */
			TYPES_INCOMPATIBLE,

			/* Trying to connect an output port with an input port
			 * that already has an incoming connection */
			INPUT_ALREADY_CONNECTED,

			/* Trying to add a connection that would introduce a
			 * cycle */
			CYCLE,

			/* Trying to connect two ports for which at least one of
			 * the node indicated incompatibility through its
			 * pre-connect function */
			PRECONNECT_FAILED

		};

		/* Current error message */
		struct {
			/* The actual error message */
			std::string message;

			/* Position in pixel coordinates */
			struct am_point position;

			/* Indicates whether the error message should be
			 * displayed */
			bool valid;
		} error;

		/* Currently dragged node if in mode is MOUSE_MODE_DRAG_NODE */
		struct {
			/* Position of the dragged node in graph coordinates at
			 * the beginning of the dragging operation */
			struct am_point startPosition;

			/* Pointer to the dragged node itself */
			struct am_dfg_node* node;
		} draggedNode;

		/* Currently dragged port if in MOUSE_MODE_DRAG_PORT */
		struct {
			/* Position of the dragged port in graph coordinates at
			 * the beginning of the dragging operation */
			struct am_point startPosition;

			/* The dragged port itself */
			struct am_dfg_port* port;

			/* Port to be disconnected from the dragged port when
			 * dragged port is connected to a new port */
			struct am_dfg_port* disconnectPort;

			/* If true, reverses start and end coordinates of the
			 * floating connection when rendering during a port drag
			 * operation */
			bool reverseFloatingConnection;

			/* Connection not to be rendered while dragging a
			 * port */
			struct am_dfg_connection ignoreConnection;
		} draggedPort;

		struct am_point currMouseGraphPos;

		/* Type registry used to check for port compatibility */
		struct am_dfg_type_registry* type_registry;

		virtual void paintErrorMessage(cairo_t* cr);
		virtual void cairoPaintEvent(cairo_t* cr);
		virtual void mouseMoveNavigate(
			const struct am_point* screen_pos);
		virtual void mouseMoveDragNode(QMouseEvent* event);
		virtual void mouseMoveDragPort(const struct am_point* screen_pos,
					       const struct am_point* graph_pos);
		virtual void mouseMoveNoMode(const struct am_point* graph_pos);
		virtual void mouseReleasePort(const struct am_point* screen_pos);

		virtual void startDragNode(struct am_dfg_node* n);
		virtual void startDragPort(struct am_dfg_port* phover,
					   const struct am_point* screen_pos,
					   const struct am_point* graph_pos,
					   bool ignoreConnection);
		virtual void abortDrag(void);

		virtual enum PortCompatibility portsCompatible(
			const struct am_dfg_port* pa,
			const struct am_dfg_port* pb,
			struct am_dfg_path* cycle,
			const struct am_dfg_port* ignore_src,
			const struct am_dfg_port* ignore_dst);

		virtual bool checkPreconnectPorts(
			const struct am_dfg_port* pa,
			const struct am_dfg_port* pb,
			std::string* error);

		void portsIncompatibilityErrorMessage(
			enum PortCompatibility compat,
			const struct am_dfg_port* pa,
			const struct am_dfg_port* pb,
			std::string& error);

		virtual void keyPressEvent(QKeyEvent* event);

	signals:
		void nodeDoubleClicked(struct am_dfg_node* n);
		void createNodeAt(struct am_dfg_graph* g, struct am_point p);
		void portsConnected(struct am_dfg_graph* g,
				    struct am_dfg_port* psrc,
				    struct am_dfg_port* pdst);

	protected slots:
		void showContextMenu(const QPoint& p);
};

#endif
