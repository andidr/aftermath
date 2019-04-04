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

#include <QCompleter>
#include <QDialog>
#include <QLineEdit>
#include <QTableView>
#include "../Exception.h"
#include "../models/DFGNodeTypeRegistryTableModel.h"

extern "C" {
	#include <aftermath/core/dfg_node.h>
	#include <aftermath/core/dfg_node_type_registry.h>
}

/* A dialog that lets the user choose from the node type of a DFG node type
 * registry. The list of types is narrowed down as the user types into a line
 * edit widget.
 *
 *  +-----------------------------------+
 *  | Select node type                  |
 *  +-----------------------------------+
 *  | Type: [_________________________] |
 *  |                                   |
 *  | +-------------------------------+ |
 *  | | Name     | Human-redable name | |
 *  | +-------------------------------+ |
 *  | |          |                    | |
 *  | |          |                    | |
 *  | |          |                    | |
 *  | +-------------------------------+ |
 *  |                                   |
 *  |                       Cancel   OK |
 *  +-----------------------------------+
 */
class DFGNodeTypeSelectionDialog : public QDialog {
	Q_OBJECT

	public:
		DFGNodeTypeSelectionDialog(struct am_dfg_node_type_registry* ntr,
					   QWidget *parent = Q_NULLPTR,
					   Qt::WindowFlags f = Qt::WindowFlags());

		virtual ~DFGNodeTypeSelectionDialog();

		struct am_dfg_node_type* getSelectedType();

	protected:
		DFGNodeTypeRegistryTableModel* model;
		QLineEdit* edit;
		QTableView* table;

		bool eventFilter(QObject* o, QEvent* e);
		bool lineEditUpDownFilter(int key);

	protected slots:
		void accepted();
		void rejected();
		void editChanged(const QString& txt);
};
