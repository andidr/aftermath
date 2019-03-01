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

#ifndef AM_GUI_AFTERMATHCONTROLLER_H
#define AM_GUI_AFTERMATHCONTROLLER_H

#include "AftermathSession.h"
#include "gui/AftermathGUI.h"
#include "gui/widgets/ManagedWidget.h"
#include "gui/widgets/DFGWidget.h"
#include "MainWindow.h"
#include <QObject>

extern "C" {
	#include <aftermath/core/dfg_node.h>
	#include <aftermath/render/cairo_extras.h>
}

/* The AftermathController handles interaction with the user through the GUI and
 * the backend. */
class AftermathController {
	public:
		AftermathController(AftermathSession* session,
				    MainWindow* mainWindow);
		virtual ~AftermathController();

		static void showError(const QString& msg,
				      QWidget* parent = Q_NULLPTR);

	protected:
		void DFGNodeDoubleClicked(struct am_dfg_node* n);
		struct am_dfg_node* createNodeAt(struct am_dfg_node_type* nt,
						 struct am_dfg_graph* g,
						 struct am_point p);
		void execCreateNodeAtAdialog(struct am_dfg_graph* g,
					     struct am_point p);
		void deleteNode(struct am_dfg_node* n);
		void deleteWidget(ManagedWidget* w);
		void deleteWidgetRec(ManagedWidget* w);
		void widgetDeletionOrder(QObject* o,
					 QList<ManagedWidget*>& list);
		void setupConnections(QWidget* w, size_t* num_connections);
		bool reparentWidget(ManagedWidget* w,
				    ManagedContainerWidget* old_parent,
				    int old_idx,
				    ManagedContainerWidget* new_parent,
				    int new_idx);

		static void portsConnected(struct am_dfg_graph* g,
					   struct am_dfg_port* psrc,
					   struct am_dfg_port* pdst);

		MainWindow* mainWindow;
		AftermathSession* session;
		std::vector<QMetaObject::Connection> connections;
};

#endif
