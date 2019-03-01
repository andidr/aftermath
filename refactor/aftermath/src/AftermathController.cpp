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
#include "gui/widgets/ToolbarButton.h"
#include "models/GUITreeModel.h"
#include "Exception.h"
#include <QMessageBox>
#include <QMetaProperty>

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

/* Creates a DFG node of type nt at position p and adds it to the graph
 * g. Returns the newly created node. If an error occurs, an exception is
 * thrown. */
struct am_dfg_node* AftermathController::createNodeAt(
	struct am_dfg_node_type* nt,
	struct am_dfg_graph* g,
	struct am_point p)
{
	struct am_dfg_node_type_registry* ntr;
	struct am_dfg_coordinate_mapping* mapping;
	struct am_dfg_node* n;
	long id;

	ntr = this->session->getDFGNodeTypeRegistry();
	mapping = session->getDFGCoordinateMapping();

	if(am_dfg_graph_generate_id(g, &id))
		throw AftermathException("Could not generate ID for new node");

	if(!(n = am_dfg_node_type_registry_instantiate(ntr, nt, id)))
		throw AftermathException("Could not instantiate node");

	if(am_dfg_graph_add_node(g, n)) {
		am_dfg_node_destroy(n);
		free(n);

		throw AftermathException("Could not add node to graph");
	}

	if(am_dfg_coordinate_mapping_set_coordinates(mapping, id, p.x, p.y)) {
		am_dfg_graph_remove_node(g, n);
		am_dfg_node_destroy(n);
		free(n);

		throw AftermathException("Could not set coordianted for node");
	}

	return n;
}

/* Called when a DFGWidget indicates that a new DFG node should be created for
 * the graph g at graph position p. Opens a dialog that lets the user select the
 * node type and creates the new node if the dialog is confirmed. */
void AftermathController::execCreateNodeAtAdialog(struct am_dfg_graph* g,
						  struct am_point p)
{
	struct am_dfg_node_type_registry* ntr;
	struct am_dfg_node_type* nt;

	ntr = session->getDFGNodeTypeRegistry();

	DFGNodeTypeSelectionDialog dlg(ntr, this->mainWindow);
	dlg.setModal(true);
	dlg.exec();

	if(dlg.result() == QDialog::Accepted) {
		nt = dlg.getSelectedType();

		try {
			this->createNodeAt(nt, g, p);
		} catch(std::exception& e) {
			this->showError(e.what());
		}
	}
}

/* Sets up the connections of a widget; The number of added connections is
 * returned in *num_connections. */
void AftermathController::setupConnections(QWidget* w, size_t* num_connections)
{
	DFGWidget* dfgWidget;

	if(w->property("DFGNode").isValid()) {
		QMetaObject::Connection c = QObject::connect(
			w,
			SIGNAL(processDFGNodeSignal(struct am_dfg_node*)),
			this->session->getDFGProcessorp(),
			SLOT(DFGNodeTriggered(struct am_dfg_node*)));

		this->connections.push_back(c);
		(*num_connections)++;
	}

	if((dfgWidget = dynamic_cast<DFGWidget*>(w))) {
		dfgWidget->setGraph(session->getDFG());
		dfgWidget->setCoordinateMapping(session->getDFGCoordinateMapping());
		dfgWidget->setTypeRegistry(session->getDFGTypeRegistry());

		QMetaObject::Connection c = QObject::connect(
			dfgWidget,
			&DFGWidget::nodeDoubleClicked,
			[&](struct am_dfg_node* n){
				this->DFGNodeDoubleClicked(n);
			});
		this->connections.push_back(c);
		(*num_connections)++;

		c = QObject::connect(
			dfgWidget,
			&DFGWidget::createNodeAt,
			[&](struct am_dfg_graph* g, struct am_point p) {
				this->execCreateNodeAtAdialog(g, p);
			});
		this->connections.push_back(c);
		(*num_connections)++;

		c = QObject::connect(dfgWidget,
				     &DFGWidget::portsConnected,
				     this->portsConnected);
		this->connections.push_back(c);
		(*num_connections)++;
	}
}

AftermathController::AftermathController(AftermathSession* session,
					 MainWindow* mainWindow)
	: mainWindow(mainWindow), session(session)
{
	AftermathGUI& gui = session->getGUI();
	qRegisterMetaType<struct am_dfg_node*>("am_dfg_node*");

	size_t num_connections = 0;

	try {
		gui.applyToWidgetsOfType<QWidget>(
			[&](QWidget* w) {
				this->setupConnections(w, &num_connections);
			});
	} catch(...) {
		auto it = this->connections.end() - num_connections;
		this->connections.erase(it, this->connections.end());
		throw;
	}

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

/* Visits the GUI tree item root and its descendants in post-order depth-first
 * search and adds each non-NULL widget associated with the items top the
 * list. */
static void widgetListDFSPostorder(GUITreeItem* root,
				   QList<ManagedWidget*>& list)
{
	GUITreeItem* child;

	if(root->getNumChildren() > 0) {
		for(size_t i = 0; i < root->getNumChildren(); i++) {
			child = root->getChild(i);
			widgetListDFSPostorder(child, list);
		}
	}

	if(root->getWidget())
		list.append(root->getWidget());
}

/* Determines the order in which the descendants of o (and o itself) that are
 * managed widgets must be deleted in order to avoid deleting parents before
 * their children. */
void AftermathController::widgetDeletionOrder(QObject* o,
					      QList<ManagedWidget*>& list)
{
	GUITreeItem root(NULL);

	GUITreeModel::buildItem(o, &root);
	widgetListDFSPostorder(&root, list);
}

/* Removes a single DFG node from the graph and destroys it. */
void AftermathController::deleteNode(struct am_dfg_node* n)
{
	struct am_dfg_graph* g = this->session->getDFG();

	am_dfg_graph_remove_node(g, n);
	am_dfg_node_destroy(n);
	free(n);
}

/* Removes a single managed widget from its parent and deletes it. */
void AftermathController::deleteWidget(ManagedWidget* w)
{
	QList<ManagedWidget*> l;
	QObject* o;
	QWidget* qw;
	struct am_dfg_node* n;

	/* Check if this is a widget with DFG node */
	if((o = dynamic_cast<QObject*>(w))) {
		QVariant propval = o->property("DFGNode");

		if(propval.isValid()) {
			if((n = propval.value<struct am_dfg_node*>()))
				this->deleteNode(n);
		}
	}

	w->unparent();

	/* Synchronize AftermathGUI */
	if((qw = dynamic_cast<QWidget*>(w)))
		this->session->getGUI().removeWidgetRec(qw);

	delete w;
}

/* Safely deletes w and all of its descendants */
void AftermathController::deleteWidgetRec(ManagedWidget* w)
{
	QList<ManagedWidget*> l;
	QObject* o;

	if((o = dynamic_cast<QObject*>(w)))
		AftermathController::widgetDeletionOrder(o, l);
	else
		l.append(w);

	for(auto w: l)
		this->deleteWidget(w);
}

/* Reparents a widget w, such that new_parent becomes its new parent. Returns
 * true if the widget's parent could successfully be changed, otherwise
 * false. */
bool AftermathController::reparentWidget(ManagedWidget* w,
					 ManagedContainerWidget* old_parent,
					 int old_idx,
					 ManagedContainerWidget* new_parent,
					 int new_idx)
{
	QWidget* qw = dynamic_cast<QWidget*>(w);

	if(!qw)
		throw AftermathException("ManagedWidget is not a QWidget");

	old_parent->removeChild(qw);

	if(old_parent == new_parent && old_idx < new_idx)
		new_idx--;

	new_parent->addChild(qw, new_idx);

	return true;
}
