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

#include "WidgetTypeListModel.h"
#include <QMimeData>
#include <algorithm>

WidgetTypeListModel::WidgetTypeListModel(
	const GUIFactory* factory, QObject *parent) :
	QAbstractTableModel(parent), factory(factory)
{
	for(auto creator: this->factory->getWidgetCreators())
		this->filteredCreators.push_back(creator);

	std::sort(this->filteredCreators.begin(),
		  this->filteredCreators.end(),
		  WidgetCreatorComparator());
}

int WidgetTypeListModel::rowCount(const QModelIndex& parent) const
{
	return this->filteredCreators.size();
}

int WidgetTypeListModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

/* Returns the row-th creator. If the row index exceeds the maximum index for
 * the list of types, NULL is returned.
 */
WidgetCreator* WidgetTypeListModel::getWidgetCreator(int row)
{
	if(row < 0 || row >= this->filteredCreators.size())
		return NULL;

	return filteredCreators[row];
}

/* Returns the string as a QVariant for the specified index. Only the display
 * role is implemented.
 */
QVariant WidgetTypeListModel::data(const QModelIndex& index, int role)
	const
{
	int row = index.row();
	int column = index.column();
	const char* gn;

	if(role == Qt::DisplayRole) {
		if(row < 0 ||
		   row >= this->filteredCreators.size() ||
		   column >= 2)
		{
			return QVariant();
		}

		if(index.column() == 0) {
			gn = this->filteredCreators[row]->getGroupName().c_str();
			return QString(gn);
		}
	}

	return QVariant();
}

/* Provides the name of the section-th column (first column: the creator's group
 * name). Only the display role is implemented and only for the horizontal
 * orientation.
 */
QVariant WidgetTypeListModel::headerData(int section,
					 Qt::Orientation orientation,
					 int role) const
{
	if(role == Qt::DisplayRole) {
		if(orientation == Qt::Horizontal) {
			if(section == 0)
				return QString("Name");
		}
	}

	return QVariant();
}

/* Filters the list of creators with a string str. The set of creators is
 * limited to creators whose group name containts the substring str. The
 * parameter sens indicates whether the search for the substring should be case
 * sensistive or case insensitive.
 */
void WidgetTypeListModel::setFilterString(
	const QString& str, enum Qt::CaseSensitivity sens)
{
	this->filteredCreators.clear();

	for(auto creator: this->factory->getWidgetCreators()) {
		const QString gname(creator->getGroupName().c_str());

		if(gname.indexOf(str, 0, sens) != -1)
			this->filteredCreators.push_back(creator);
	}

	std::sort(this->filteredCreators.begin(),
		  this->filteredCreators.end(),
		  WidgetCreatorComparator());

	emit layoutChanged();
}


Qt::ItemFlags WidgetTypeListModel::flags(const QModelIndex& index) const
{
	if(!index.isValid())
		return 0;

	return QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;
}

QStringList WidgetTypeListModel::mimeTypes() const
{
	QStringList types;
	types << "application/aftermath.widgetgroupname";

	return types;
}

Qt::DropActions WidgetTypeListModel::supportedDragActions() const
{
	return Qt::CopyAction;
}

QMimeData* WidgetTypeListModel::mimeData(const QModelIndexList& indexes) const
{
	QMimeData* mimeData = NULL;
	WidgetCreator* c;

	try {
		mimeData = new QMimeData();
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);

		for(const auto& idx: indexes) {
			if (idx.isValid()) {
				c = this->filteredCreators[idx.row()];
				stream << QString(c->getGroupName().c_str());
			}
		}

		mimeData->setData("application/aftermath.widgetgroupname",
				  encodedData);
	} catch(...) {
		delete mimeData;
		throw;
	}

	return mimeData;
}
