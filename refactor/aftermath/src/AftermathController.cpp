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

#include "AftermathController.h"
#include "gui/DFGNodePropertyDialog.h"
#include "gui/DFGNodeTypeSelectionDialog.h"
#include "gui/widgets/CairoWidgetWithDFGNode.h"
#include "gui/widgets/HierarchyComboBox.h"
#include <QMessageBox>

/* Called when two ports are connected. */
void AftermathController::portsConnected(struct am_dfg_graph* g,
					 struct am_dfg_port* psrc,
					 struct am_dfg_port* pdst)
{
	am_dfg_port_mask_reset(&psrc->node->required_mask);
	psrc->node->required_mask.pull_old |=
		am_dfg_port_mask_bits(psrc);
	am_dfg_schedule_component(psrc->node);
}

/* Shows an error message with parent as the parent widget. */
void AftermathController::showError(const QString& msg, QWidget* parent)
{
	QMessageBox mb(parent);
	mb.setText(msg);
	mb.exec();
}

/* Called when a DFGWidget indicates that a new DFG node should be created for
 * the graph g at graph position p. Opens a dialog that lets the user select the
 * node type and creates the new node if the dialog is confirmed. */
void AftermathController::execCreateNodeAtAdialog(struct am_dfg_graph* g,
						  struct am_point p)
{
	struct am_dfg_node_type_registry* ntr;
	struct am_dfg_node* n;
	struct am_dfg_node_type* nt;
	struct am_dfg_coordinate_mapping* mapping;
	long id;

	ntr = session->getDFGNodeTypeRegistry();
	mapping = session->getDFGCoordinateMapping();

	DFGNodeTypeSelectionDialog dlg(ntr, this->mainWindow);
	dlg.setModal(true);
	dlg.exec();

	if(dlg.result() == QDialog::Accepted) {
		nt = dlg.getSelectedType();

		if(am_dfg_graph_generate_id(g, &id)) {
			showError("Could not generate ID for new node");
			return;
		}

		if(!(n = am_dfg_node_type_registry_instantiate(ntr, nt, id))) {
			showError("Could not instantiate node");
			return;
		}

		if(am_dfg_graph_add_node(g, n)) {
			am_dfg_node_destroy(n);
			free(n);
			showError("Could not add node to graph");
			return;
		}

		if(am_dfg_coordinate_mapping_set_coordinates(mapping, id, p.x, p.y)) {
			am_dfg_graph_remove_node(g, n);
			am_dfg_node_destroy(n);
			free(n);
			showError("Could not set coordianted for node");
			return;
		}
	}
}

AftermathController::AftermathController(AftermathSession* session,
					 MainWindow* mainWindow)
	: mainWindow(mainWindow), session(session)
{
	AftermathGUI& gui = session->getGUI();

	/* Set up DFG widgets */
	gui.applyToWidgetsOfType<DFGWidget>(
		[&](DFGWidget* w) {
			QMetaObject::Connection c;

			w->setGraph(session->getDFG());
			w->setCoordinateMapping(session->getDFGCoordinateMapping());
			w->setTypeRegistry(session->getDFGTypeRegistry());

			c = QObject::connect(
				w, &DFGWidget::nodeDoubleClicked,
				[&](struct am_dfg_node* n){
					this->DFGNodeDoubleClicked(n);
				});
			this->connections.push_back(c);

			c = QObject::connect(
				w,
				&DFGWidget::createNodeAt,
				[&](struct am_dfg_graph* g, struct am_point p) {
					this->execCreateNodeAtAdialog(g, p);
				});
			this->connections.push_back(c);

			c = QObject::connect(w, &DFGWidget::portsConnected,
					    this->portsConnected);
			this->connections.push_back(c);
		});

	/* Set up widgets that can trigger re-evaluation of a DFG node */
	gui.applyToWidgetsOfType<CairoWidgetWithDFGNode>(
		[&](CairoWidgetWithDFGNode* w) {
			QMetaObject::Connection c;

			c = QObject::connect(
				w,
				&CairoWidgetWithDFGNode::processDFGNodeSignal,
				session->getDFGProcessorp(),
				&DFGQTProcessor::DFGNodeTriggered);
			this->connections.push_back(c);
		});

	gui.applyToWidgetsOfType<HierarchyComboBox>(
		[&](HierarchyComboBox* w) {
			QMetaObject::Connection c;

			c = QObject::connect(
				w,
				&HierarchyComboBox::processDFGNodeSignal,
				session->getDFGProcessorp(),
				&DFGQTProcessor::DFGNodeTriggered);
			this->connections.push_back(c);
		});

	/* Set root widget */
	try {
		this->mainWindow->setCentralWidget(gui.getRootWidget());
		gui.markWidgetBound(gui.getRootWidget());
	} catch(...) {
		if(gui.getRootWidget()->parent())
			gui.markWidgetBound(gui.getRootWidget());
	}
}

AftermathController::~AftermathController()
{
	/* Disconnect all connections made by the controller */
	for(auto& connection: this->connections)
		QObject::disconnect(connection);
}

/* Called when the user double-clicked on a DFG node n. Shows a dialog for
 * editing the node's properties. If the node doesn't have any properties
 * nothing happens.
 */
void AftermathController::DFGNodeDoubleClicked(struct am_dfg_node* n)
{
	if(n->type->num_properties == 0)
		return;

	DFGNodePropertyDialog dlg(n, this->mainWindow);
	dlg.setModal(true);
	dlg.exec();

	if(dlg.result() == QDialog::Accepted) {
		/* The changed attributes might have an impact on the output
		 * values. Pull all input ports and indicate that new data is
		 * available at all output ports.
		 *
		 * FIXME: This might be a bit too brutal: not all output ports
		 * might actually provide new data and not all input data might
		 * be required to generate new values. Also, by externally
		 * setting output ports to new, the process function of the node
		 * might get confused, since it might be the case that this
		 * pattern never occurs when evaluating the port dependencies.
		 */
		am_dfg_port_mask_reset(&n->required_mask);
		n->required_mask.push_new =
			am_dfg_node_type_output_mask(n->type);
		n->required_mask.pull_old =
			am_dfg_node_type_input_mask(n->type);
		am_dfg_schedule_component(n);
	}
}
