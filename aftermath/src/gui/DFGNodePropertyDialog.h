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

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <map>
#include "../Exception.h"

extern "C" {
	#include <aftermath/core/dfg_node.h>
}

/* A dialog that lets the user edit all properties of a DFG node using QLineEdit
 * widgets. When accepted, the dialog modifies the properties. If rejected, all
 * properties remain untouched.
 */
class DFGNodePropertyDialog : public QDialog {
	Q_OBJECT

	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg)
				{ }
		};

		DFGNodePropertyDialog(struct am_dfg_node* n,
				      QWidget *parent = Q_NULLPTR,
				      Qt::WindowFlags f = Qt::WindowFlags());

		virtual ~DFGNodePropertyDialog();

	protected:
		struct am_dfg_node* dfgNode;
		std::map<struct am_dfg_property*, QLineEdit*> propEditMap;
		QLabel* messageLabel;

		void propertyChanged(struct am_dfg_property* p);
		int checkAllProperties();
		int checkProperty(struct am_dfg_property* p);
		void commitAllProperties();
		void commitProperty(struct am_dfg_property* p);

	protected slots:
		void accepted();
		void rejected();
};
