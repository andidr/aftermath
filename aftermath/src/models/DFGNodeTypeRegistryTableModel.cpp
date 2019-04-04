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

#include "DFGNodeTypeRegistryTableModel.h"

DFGNodeTypeRegistryTableModel::DFGNodeTypeRegistryTableModel(
	struct am_dfg_node_type_registry* ntr,
	QObject *parent) :
	QAbstractTableModel(parent), ntr(ntr)
{
	struct am_dfg_node_type* t;

	am_dfg_node_type_registry_for_each_type(ntr, t)
		this->filteredTypes.push_back(t);
}

int DFGNodeTypeRegistryTableModel::rowCount(const QModelIndex& parent) const
{
	return this->filteredTypes.size();
}

int DFGNodeTypeRegistryTableModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

/* Returns the row-th type. If the row index exceeds the maximum index for the
 * list of types, NULL is returned.
 */
struct am_dfg_node_type* DFGNodeTypeRegistryTableModel::getType(int row)
{
	if(row < 0 || row >= this->filteredTypes.size())
		return NULL;

	return filteredTypes[row];
}

/* Returns the string as a QVariant for the specified index (first column: the
 * type name, second column: the human-redable name). Only the display role is
 * implemented.
 */
QVariant DFGNodeTypeRegistryTableModel::data(const QModelIndex& index, int role)
	const
{
	int row = index.row();
	int column = index.column();

	if(role == Qt::DisplayRole) {
		if(row < 0 || row >= this->filteredTypes.size() || column >= 2)
			return QVariant();

		if(index.column() == 0)
			return QString(this->filteredTypes[row]->name);
		else
			return QString(this->filteredTypes[row]->hrname);
	}

	return QVariant();
}

/* Provides the name of the section-th column (first column: the type name,
 * second column: the human-redable name). Only the display role is implemented
 * and only for the horizontal orientation.
 */
QVariant DFGNodeTypeRegistryTableModel::headerData(int section,
						   Qt::Orientation orientation,
						   int role) const
{
	if(role == Qt::DisplayRole) {
		if(orientation == Qt::Horizontal) {
			if(section == 0)
				return QString("Name");
			else if(section == 1)
				return QString("Human-readable name");
		}
	}

	return QVariant();
}

/* Filters the list of types with a string str. The set of types is limited to
 * types whose name or human-redable name containts the substring str. The
 * parameter sens indicates whether the search for the substring should be case
 * sensistive or case insensitive.
 */
void DFGNodeTypeRegistryTableModel::setFilterString(
	const QString& str, enum Qt::CaseSensitivity sens)
{
	struct am_dfg_node_type* t;

	this->filteredTypes.clear();

	am_dfg_node_type_registry_for_each_type(ntr, t) {
		if(QString(t->name).indexOf(str, 0, sens) != -1 ||
		   QString(t->hrname).indexOf(str, 0, sens) != -1)
		{
			this->filteredTypes.push_back(t);
		}
	}

	emit layoutChanged();
}
