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

#include "DFGWidget.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QWheelEvent>

DFGWidget::DFGWidget(QWidget* parent)
	: super(parent),
	  graph(NULL),
	  mouseMode(MOUSE_MODE_NONE)
{
	am_dfg_path_init(&this->cycle);
	am_dfg_renderer_init(&this->renderer);

	this->setMouseTracking(true);
	this->setCursor(Qt::OpenHandCursor);

	this->params.error.font = "Sans";
	this->params.error.font_size = 15;
	this->params.error.color = AM_RGBA255(0xFF, 0x00, 0x00, 0xFF);
	this->params.error.bgcolor = AM_RGBA255(0xFF, 0xFF, 0xFF, 0xFF);
	this->params.error.line_color = AM_RGBA255(0xFF, 0x00, 0x00, 0xFF);

	this->params.error.padding_x = 5;
	this->params.error.padding_y = 5;

	this->params.error.margin_x = 10;
	this->params.error.margin_y = 10;

	this->error.valid = false;

	this->setFocusPolicy(Qt::ClickFocus);
}

/* Opens a file save dialog and writes the current graph to the selected
 * file. */
void DFGWidget::saveGraph()
{
	QString filename = QFileDialog::getSaveFileName(
		this,
		tr("Save graph to file"),
		"",
		tr("Aftermath data flow graphs (*.dfg)"));

	/* Dialog cancelled by the user? */
	if(filename.isNull())
		return;

	this->saveGraph(filename);
}

/* Saves the current graph in object notation to the file identified by its file
 * name. If no graph is associated to the widget, an empty graph is written.
 */
void DFGWidget::saveGraph(const QString& filename)
{
	struct am_dfg_graph emptyGraph;
	struct am_dfg_graph* g;
	struct am_object_notation_node* ograph;
	int do_throw = 1;
	QByteArray barr;

	barr = filename.toUtf8();

	if(!this->graph) {
		am_dfg_graph_init(&emptyGraph, 0);
		g = &emptyGraph;
	} else {
		g = this->graph;
	}

	if(!(ograph = am_dfg_coordinate_mapping_graph_to_object_notation(
		     this->coordinate_mapping, g)))
	{
		goto out;
	}

	if(am_object_notation_save(ograph, barr.data()))
		goto out_objnot;

	do_throw = 0;

out_objnot:
	am_object_notation_node_destroy(ograph);
	free(ograph);
out:
	if(g == &emptyGraph)
		am_dfg_graph_destroy(&emptyGraph);

	if(do_throw)
		throw Exception("Could not save graph");
}

/**
 * Paint the current error message (if valid) on the associated position in a
 * filled rectangle.
 */
void DFGWidget::paintErrorMessage(cairo_t* cr)
{
	cairo_text_extents_t extents;
	struct am_rect rect;

	if(!this->error.valid)
		return;

	cairo_select_font_face(cr,
			       this->params.error.font,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);

	cairo_set_font_size(cr, this->params.error.font_size);

	/* Determine size of the surrounding box */
	cairo_text_extents(cr, this->error.message.c_str(), &extents);

	rect.x = this->error.position.x + this->params.error.margin_x;
	rect.y = this->error.position.y + this->params.error.margin_y;
	rect.width = extents.width + 2 * this->params.error.padding_x;
	rect.height = extents.height + 2 * this->params.error.padding_y;

	/* Draw box background */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(this->params.error.bgcolor));
	cairo_rectangle(cr, AM_RECT_ARGS(rect));
	cairo_fill(cr);

	/* Draw box foreground */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(this->params.error.line_color));
	cairo_rectangle(cr, AM_RECT_ARGS(rect));
	cairo_stroke(cr);

	/* Draw text of the error message */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(this->params.error.color));
	cairo_move_to(cr,
		      rect.x + this->params.error.padding_x,
		      rect.y + this->params.error.padding_y + extents.height);

	cairo_show_text(cr, this->error.message.c_str());
}

void DFGWidget::cairoPaintEvent(cairo_t* cr)
{
	am_dfg_renderer_render(&this->renderer, cr);
	this->paintErrorMessage(cr);
}

void DFGWidget::resizeEvent(QResizeEvent* event)
{
	super::resizeEvent(event);

	am_dfg_renderer_set_width(&this->renderer, width());
	am_dfg_renderer_set_height(&this->renderer, height());
}

/**
 * Set the graph to be rendered by the widget
 */
void DFGWidget::setGraph(struct am_dfg_graph* g)
{
	am_dfg_renderer_set_graph(&this->renderer, g);
	this->graph = g;
	this->update();
}

/**
 * Set the graph to be rendered by the widget
 */
void DFGWidget::setCoordinateMapping(struct am_dfg_coordinate_mapping* m)
{
	am_dfg_renderer_set_coordinate_mapping(&this->renderer, m);
	this->coordinate_mapping = m;
	this->update();
}

/**
 * Sets the type registry to be used for type checks
 */
void DFGWidget::setTypeRegistry(struct am_dfg_type_registry* tr)
{
	this->type_registry = tr;
}

/**
 * Called when the mouse is moved in navigation mode
 */
void DFGWidget::mouseMoveNavigate(const struct am_point* screen_pos)
{
	double dx;
	double dy;
	double gdx;
	double gdy;
	double gx;
	double gy;

	dx = this->mouseStartPos.x - screen_pos->x;
	dy = this->mouseStartPos.y - screen_pos->y;

	gdx = am_dfg_renderer_screen_w_to_graph(&this->renderer, dx);
	gdy = am_dfg_renderer_screen_h_to_graph(&this->renderer, dy);

	gx = this->graphStartPos.x + gdx;
	gy = this->graphStartPos.y + gdy;

	am_dfg_renderer_set_offset(&this->renderer, gx, gy);

	this->update();
}

/**
 * Called when the mouse is moved while dragging a node
 */
void DFGWidget::mouseMoveDragNode(QMouseEvent* event)
{
	double dx;
	double dy;
	double gdx;
	double gdy;
	double gx;
	double gy;

	dx = event->x() - this->mouseStartPos.x;
	dy = event->y() - this->mouseStartPos.y;

	gdx = am_dfg_renderer_screen_w_to_graph(&this->renderer, dx);
	gdy = am_dfg_renderer_screen_h_to_graph(&this->renderer, dy);

	gx = this->draggedNode.startPosition.x + gdx;
	gy = this->draggedNode.startPosition.y + gdy;

	am_dfg_coordinate_mapping_set_coordinates(this->coordinate_mapping,
						  this->draggedNode.node->id,
						  gx, gy);

	this->update();
}

/**
 * Checks if two ports are compatible, i.e., if they could be connected. If a
 * connection of the two ports would create a cycle and if cycle != NULL, the
 * variable is used to store the cycle that would be created. If ignore_a and
 * ignore_b are non-NULL, the edge (ignore_src, ignore_dst) is excluded from the
 * cycle check.
 */
enum DFGWidget::PortCompatibility
DFGWidget::portsCompatible(const struct am_dfg_port* pa,
			   const struct am_dfg_port* pb,
			   struct am_dfg_path* cycle = NULL,
			   const struct am_dfg_port* ignore_src = NULL,
			   const struct am_dfg_port* ignore_dst = NULL)
{
	const struct am_dfg_port* psrc = NULL;
	const struct am_dfg_port* pdst = NULL;

	/* Cannot connect two input ports */
	if(am_dfg_port_is_input_port(pa) && am_dfg_port_is_input_port(pb))
		return PortCompatibility::INPUT_TO_INPUT;

	/* Cannot connect two output ports */
	if(am_dfg_port_is_output_port(pa) && am_dfg_port_is_output_port(pb))
		return PortCompatibility::OUTPUT_TO_OUTPUT;

	if(!this->type_registry)
		throw DFGWidgetException();

	am_dfg_sort_ports_inout(pa, pb, &psrc, &pdst);

	/* Input ports cannot have more than one connection. Do not generate
	 * error if the ports are currently already connected. */
	if(am_dfg_port_is_connected(pdst) && !am_dfg_ports_connected(psrc, pdst))
		return PortCompatibility::INPUT_ALREADY_CONNECTED;

	if(!am_dfg_type_registry_types_compatible(this->type_registry,
						  psrc->type->type,
						  pdst->type->type))
	{
		return PortCompatibility::TYPES_INCOMPATIBLE;
	}

	if(!this->graph)
		throw DFGWidgetException();

	if(am_dfg_graph_has_cycle(this->graph,
				  psrc,
				  pdst,
				  ignore_src,
				  ignore_dst,
				  cycle))
	{
		return PortCompatibility::CYCLE;
	}

	return PortCompatibility::COMPATIBLE;
}

/**
 * Generates an error message for the compatibility result for two ports pa and
 * pb. The generated message is passed in error.
 */
void DFGWidget::portsIncompatibilityErrorMessage(
	enum PortCompatibility compat,
	const struct am_dfg_port* pa,
	const struct am_dfg_port* pb,
	std::string& error)
{
	switch(compat) {
		case OUTPUT_TO_OUTPUT:
			error = "Cannot connect two output ports";
			break;
		case INPUT_TO_INPUT:
			error = "Cannot connect two input ports";
			break;
		case TYPES_INCOMPATIBLE:
			error = std::string("Cannot convert type '") +
				pa->type->type->name +
				"' into '" +
				pb->type->type->name +
				"'";
			break;
		case INPUT_ALREADY_CONNECTED:
			error = "Input port already connected";
			break;
		case CYCLE:
			error = "This would create a cycle";
			break;
		case COMPATIBLE:
			break;
	}
}

/**
 * Called when the mouse is moved while dragging a port.
 */
void DFGWidget::mouseMoveDragPort(const struct am_point* screen_pos,
				  const struct am_point* graph_pos)
{
	struct am_dfg_port* phover;
	struct am_point float_pos = *graph_pos;
	const struct am_point* src_pos;
	const struct am_point* dst_pos;
	enum PortCompatibility compat;
	bool setCycle = false;

	this->error.valid = false;

	/* Check if mouse is hovering over another port */
	if((phover = am_dfg_renderer_port_at(&this->renderer,
					this->cairo_context,
					graph_pos->x,
					graph_pos->y)))
	{
		/* Snap to port coordinates */
		am_dfg_renderer_port_connection_point_xy(&this->renderer,
							 this->cairo_context,
							 phover,
							 &float_pos);

		compat = this->portsCompatible(this->draggedPort.port,
					       phover,
					       &this->cycle,
					       this->draggedPort.ignoreConnection.src,
					       this->draggedPort.ignoreConnection.dst);

		if(compat != COMPATIBLE) {
			this->portsIncompatibilityErrorMessage(
				compat,
				phover,
				this->draggedPort.port,
				this->error.message);

			this->error.valid = true;
			this->error.position = *screen_pos;
			setCycle = (compat == CYCLE);
		}
	}

	am_dfg_renderer_set_highlighted_port(&this->renderer, phover);

	if(setCycle) {
		/* Display the error cycle, which includes the hypothetical edge
		 * between the dragged port and the port below the mouse
		 */
		am_dfg_renderer_set_error_cycle(&this->renderer, &this->cycle);
		am_dfg_renderer_unset_floating_connection(&this->renderer);
	} else {
		am_dfg_renderer_unset_error_cycle(&this->renderer);

		if(this->draggedPort.reverseFloatingConnection) {
			src_pos = &this->draggedPort.startPosition;
			dst_pos = graph_pos;
		} else {
			src_pos = graph_pos;
			dst_pos = &this->draggedPort.startPosition;
		}

		am_dfg_renderer_set_floating_connection(
			&this->renderer,
			src_pos->x,
			src_pos->y,
			dst_pos->x,
			dst_pos->y);
	}

	this->update();
}

/**
 * Called when the mouse is moved without dragging anything.
 */
void DFGWidget::mouseMoveNoMode(const struct am_point* graph_pos)
{
	struct am_dfg_connection c;

	if(am_dfg_renderer_has_highlighted_connection(&this->renderer)) {
		am_dfg_renderer_unset_highlighted_connection(&this->renderer);
		this->update();
	}

	if(am_dfg_renderer_node_at(&this->renderer,
				   this->cairo_context,
				   graph_pos->x,
				   graph_pos->y))
	{
		this->setCursor(Qt::DragMoveCursor);
	} else if(am_dfg_renderer_port_at(&this->renderer,
					  this->cairo_context,
					  graph_pos->x,
					  graph_pos->y))
	{
		this->setCursor(Qt::CrossCursor);
	} else if(am_dfg_renderer_connection_at(&this->renderer,
						this->cairo_context,
						graph_pos->x,
						graph_pos->y,
						&c.src,
						&c.dst))
	{
		am_dfg_renderer_set_highlighted_connection(&this->renderer,
							   c.src, c.dst);
		this->update();
	} else {
		this->setCursor(Qt::OpenHandCursor);
	}
}

void DFGWidget::mouseMoveEvent(QMouseEvent* event)
{
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->graph)
		return;

	am_dfg_renderer_screen_to_graph(&this->renderer,
					&screen_pos,
					&this->currMouseGraphPos);

	if(this->mouseMode == MOUSE_MODE_NAVIGATE)
		this->mouseMoveNavigate(&screen_pos);
	else if(this->mouseMode == MOUSE_MODE_DRAG_NODE)
		this->mouseMoveDragNode(event);
	else if(this->mouseMode == MOUSE_MODE_DRAG_PORT)
		this->mouseMoveDragPort(&screen_pos, &this->currMouseGraphPos);
	else
		this->mouseMoveNoMode(&this->currMouseGraphPos);
}

/**
 * Called when the mouse button was just pressed while hovering over a node
 */
void DFGWidget::startDragNode(struct am_dfg_node* n)
{
	this->draggedNode.node = n;
	this->mouseMode = MOUSE_MODE_DRAG_NODE;

	am_dfg_renderer_set_selected_node(&this->renderer,
					  this->draggedNode.node);
	am_dfg_renderer_unset_selected_connection(&this->renderer);

	am_dfg_renderer_get_node_coordinate_def(
		&this->renderer, n, &this->draggedNode.startPosition);
}

/**
 * Called when the mouse button was just pressed while hovering over a port. The
 * default behavior of modifying an existing connection if the dragged is
 * already connected can be overriden by setting ignoreConnected to true.
 */
void DFGWidget::startDragPort(struct am_dfg_port* phover,
			      const struct am_point* screen_pos,
			      const struct am_point* graph_pos,
			      bool ignoreConnection)
{
	am_dfg_renderer_unset_selected_node(&this->renderer);

	if(!ignoreConnection && am_dfg_port_is_connected(phover)) {
		if(am_dfg_port_is_input_port(phover)) {
			this->draggedPort.ignoreConnection.src =
				phover->connections[0];
			this->draggedPort.ignoreConnection.dst = phover;
		} else {
			this->draggedPort.ignoreConnection.src = phover;
			this->draggedPort.ignoreConnection.dst =
				phover->connections[0];
		}

		this->mouseMode = MOUSE_MODE_DRAG_PORT;
		this->draggedPort.port = phover->connections[0];
		this->draggedPort.disconnectPort = phover;

		am_dfg_renderer_set_ignore_connection(
			&this->renderer,
			this->draggedPort.ignoreConnection.src,
			this->draggedPort.ignoreConnection.dst);

		this->draggedPort.reverseFloatingConnection =
			am_dfg_port_is_input_port(phover);
	} else {
		this->mouseMode = MOUSE_MODE_DRAG_PORT;
		this->draggedPort.port = phover;
		this->draggedPort.disconnectPort = NULL;

		this->draggedPort.reverseFloatingConnection =
			am_dfg_port_is_output_port(phover);
	}

	am_dfg_renderer_port_connection_point_xy(
		&this->renderer,
		this->cairo_context,
		this->draggedPort.port,
		&this->draggedPort.startPosition);

	this->mouseMoveDragPort(screen_pos, graph_pos);
	this->setCursor(Qt::CrossCursor);
}

void DFGWidget::mousePressEvent(QMouseEvent* event)
{
	struct am_dfg_node* n;
	struct am_dfg_port* p;
	struct am_point graph_pos;
	struct am_dfg_connection c;
	bool ignoreConnection = false;
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->graph)
		return;

	/* Right button cancels any current action */
	if(event->button() == Qt::RightButton) {
		this->abortDrag();
		am_dfg_renderer_unset_selected_connection(&this->renderer);
		am_dfg_renderer_unset_selected_node(&this->renderer);
		this->update();

		return;
	}

	/* Only react to left mouse button */
	if(event->button() != Qt::LeftButton)
		return;

	this->mouseStartPos.x = event->pos().x();
	this->mouseStartPos.y = event->pos().y();

	am_dfg_renderer_screen_to_graph(&this->renderer,
					&screen_pos,
					&graph_pos);

	/* Find out what is under the cursor */
	if((n = am_dfg_renderer_node_at(&this->renderer,
					this->cairo_context,
					graph_pos.x,
					graph_pos.y)))
	{
		this->startDragNode(n);
	} else if((p = am_dfg_renderer_port_at(&this->renderer,
					       this->cairo_context,
					       graph_pos.x,
					       graph_pos.y)))
	{
		ignoreConnection = (event->modifiers() & Qt::ShiftModifier);
		this->startDragPort(p, &screen_pos, &graph_pos,
				    ignoreConnection);
	} else if(am_dfg_renderer_connection_at(&this->renderer,
						this->cairo_context,
						graph_pos.x,
						graph_pos.y,
						&c.src,
						&c.dst))
	{
		am_dfg_renderer_unset_highlighted_connection(&this->renderer);
		am_dfg_renderer_unset_selected_node(&this->renderer);
		am_dfg_renderer_set_selected_connection(&this->renderer,
							c.src, c.dst);
		this->update();
	} else {
		am_dfg_renderer_get_offset(&this->renderer,
					   &this->graphStartPos.x,
					   &this->graphStartPos.y);
		this->setCursor(Qt::ClosedHandCursor);
		this->mouseMode = MOUSE_MODE_NAVIGATE;
	}
}

/* Called when the mouse button is released when dragging a port. */
void DFGWidget::mouseReleasePort(const struct am_point* screen_pos)
{
	struct am_dfg_port* pin;
	struct am_dfg_port* pout;
	struct am_dfg_port* p;
	struct am_point graph_pos;

	am_dfg_renderer_screen_to_graph(&this->renderer,
					screen_pos,
					&graph_pos);

	p = am_dfg_renderer_port_at(&this->renderer,
				    this->cairo_context,
				    graph_pos.x,
				    graph_pos.y);

	if(p) {
		if(this->portsCompatible(this->draggedPort.port, p) !=
		   COMPATIBLE)
		{
			return;
		}

		if(am_dfg_sort_ports_inout(this->draggedPort.port, p,
					   (struct am_dfg_port const**)&pin,
					   (struct am_dfg_port const**)&pout))
		{
			return;
		}

		/* Do nothing for already connected ports */
		if(am_dfg_ports_connected(pout, pin))
			return;

		if(this->draggedPort.disconnectPort) {
			am_dfg_port_disconnect(this->draggedPort.port,
					       this->draggedPort.disconnectPort);
		}

		am_dfg_graph_connectp(this->graph, pin, pout);
	} else {
		if(this->draggedPort.disconnectPort) {
			am_dfg_port_disconnect(this->draggedPort.port,
					       this->draggedPort.disconnectPort);
		}
	}
}

/* Aborts the current dragging operation */
void DFGWidget::abortDrag(void)
{
	this->mouseMode = MOUSE_MODE_NONE;
	this->setCursor(Qt::OpenHandCursor);
	this->error.valid = false;

	this->draggedPort.ignoreConnection.src = NULL;
	this->draggedPort.ignoreConnection.dst = NULL;

	am_dfg_renderer_unset_floating_connection(&this->renderer);
	am_dfg_renderer_unset_highlighted_port(&this->renderer);
	am_dfg_renderer_unset_error_cycle(&this->renderer);
	am_dfg_renderer_unset_ignore_connection(&this->renderer);

	this->update();
}

void DFGWidget::mouseReleaseEvent(QMouseEvent* event)
{
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->graph)
		return;

	if(this->mouseMode == MOUSE_MODE_DRAG_PORT)
		this->mouseReleasePort(&screen_pos);

	this->abortDrag();
}

void DFGWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	struct am_point graph_pos;
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};
	struct am_dfg_node* n;

	am_dfg_renderer_screen_to_graph(&this->renderer,
					&screen_pos,
					&graph_pos);

	/* Find out what is under the cursor */
	if((n = am_dfg_renderer_node_at(&this->renderer,
					this->cairo_context,
					graph_pos.x,
					graph_pos.y)))
	{
		emit nodeDoubleClicked(n);
	} else {
		emit createNodeAt(this->graph, graph_pos);
	}
}

void DFGWidget::wheelEvent(QWheelEvent* event)
{
	struct am_point p = {
		(double)event->x(),
		(double)event->y()
	};

	if(event->delta() > 0)
		am_dfg_renderer_zoom_in(&this->renderer, &p);
	else
		am_dfg_renderer_zoom_out(&this->renderer, &p);

	this->update();
}

void DFGWidget::keyPressEvent(QKeyEvent* event)
{
	struct am_dfg_connection c;
	struct am_dfg_node* n;

	if(!this->graph)
		return;

	if(event->key() == Qt::Key_Delete) {
		if(am_dfg_renderer_has_selected_connection(&this->renderer)) {
			am_dfg_renderer_get_selected_connection(&this->renderer,
								&c.src,
								&c.dst);
			am_dfg_port_disconnect(c.src, c.dst);
			am_dfg_renderer_unset_selected_connection(
				&this->renderer);
		} else if(am_dfg_renderer_has_selected_node(&this->renderer)) {
			n = am_dfg_renderer_get_selected_node(&this->renderer);
			am_dfg_graph_remove_node(this->graph, n);
			am_dfg_node_destroy(n);
			free(n);
		}

		this->update();
	} else if(event->key() == Qt::Key_N) {
		struct am_point p;

		if(this->underMouse())
			p = this->currMouseGraphPos;
		else
			am_dfg_renderer_graph_center(&this->renderer, &p);

		emit createNodeAt(this->graph, p);
	} else {
		super::keyPressEvent(event);
	}
}

DFGWidget::~DFGWidget()
{
	am_dfg_path_destroy(&this->cycle);
	am_dfg_renderer_destroy(&this->renderer);
}
