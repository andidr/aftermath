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

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStringListModel>

extern "C" {
	#include <aftermath/core/dfg_node.h>
	#include <aftermath/core/dfg_node_type_registry.h>
}

/* A two-column table model providing read-only access to all node types of a
 * DFG node type registry. The first column corresponds to the node type names
 * and the second column indicates the human-readable names.
 *
 * The types might be filtered with a string, limiting the set of types to those
 * types that contain the string as a substring.
 */
class DFGNodeTypeRegistryTableModel : public QAbstractTableModel {
	Q_OBJECT

	public:
		DFGNodeTypeRegistryTableModel(
			struct am_dfg_node_type_registry* ntr,
			QObject* parent = Q_NULLPTR);

		int rowCount(const QModelIndex& parent = QModelIndex())
			const ;

		int columnCount(const QModelIndex& parent = QModelIndex())
			const;

		QVariant data(const QModelIndex& index,
			      int role = Qt::DisplayRole) const;

		QVariant headerData(int section,
				    Qt::Orientation orientation,
				    int role) const;

		void setFilterString(
			const QString& str,
			enum Qt::CaseSensitivity sens = Qt::CaseSensitive);

		struct am_dfg_node_type* getType(int row);

	protected:
		struct am_dfg_node_type_registry* ntr;
		QVector<struct am_dfg_node_type*> filteredTypes;
};
