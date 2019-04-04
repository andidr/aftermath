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

#ifndef AM_WIDGETTYPELISTMODEL_H
#define AM_WIDGETTYPELISTMODEL_H

#include "../gui/factory/GUIFactory.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStringListModel>

/* A table model providing read-only access to all widget types for which a
 * widget creator has been registered at the indicated widget factory.
 *
 * The types might be filtered with a string, limiting the set of types to those
 * types that contain the string as a substring.
 */
class WidgetTypeListModel : public QAbstractTableModel {
	Q_OBJECT

	public:
		WidgetTypeListModel(const GUIFactory* factory,
				    QObject* parent = Q_NULLPTR);

		int rowCount(const QModelIndex& parent = QModelIndex())
			const override;

		int columnCount(const QModelIndex& parent = QModelIndex())
			const override;

		QVariant data(const QModelIndex& index,
			      int role = Qt::DisplayRole) const override;

		QVariant headerData(int section,
				    Qt::Orientation orientation,
				    int role) const override;

		void setFilterString(
			const QString& str,
			enum Qt::CaseSensitivity sens = Qt::CaseSensitive);

		WidgetCreator* getWidgetCreator(int row);

		Qt::ItemFlags flags(const QModelIndex& index) const override;
		QStringList mimeTypes() const override;
		QMimeData* mimeData(const QModelIndexList& indexes) const override;
		Qt::DropActions supportedDragActions() const override;

	protected:
		const GUIFactory* factory;
		QVector<WidgetCreator*> filteredCreators;
};

#endif
