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

#ifndef AM_GUITREEMODEL_H
#define AM_GUITREEMODEL_H

#include "GUITreeItem.h"
#include "../gui/widgets/ManagedWidget.h"
#include "../Exception.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <functional>

using WidgetReparenter = std::function<bool (ManagedWidget*,
					     ManagedContainerWidget*,
					     int,
					     ManagedContainerWidget*,
					     int)>;

using WidgetInserter = std::function<bool (ManagedContainerWidget*,
					   size_t,
					   const QString&)>;

using WidgetDeleter = std::function<bool (ManagedWidget*)>;

/* A tree model representing parent-child-relationships of managed widgets */
class GUITreeModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg) { };
		};

		explicit GUITreeModel(WidgetReparenter* reparenter = NULL,
				      WidgetInserter* inserter = NULL,
				      WidgetDeleter* deleter = NULL,
				      QObject* parent = NULL);
		~GUITreeModel();

		QVariant data(const QModelIndex& index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex& index) const override;
		QVariant headerData(int section, Qt::Orientation orientation,
				    int role = Qt::DisplayRole) const override;
		QModelIndex index(GUITreeItem* i);
		QModelIndex index(int row, int column,
				  const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex& index) const override;
		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		Qt::DropActions supportedDropActions() const override;
		Qt::DropActions supportedDragActions() const override;

		bool removeRows(int row,
				int count,
				const QModelIndex& parentIndex = QModelIndex())
			override;

		bool moveRows(const QModelIndex& srcParentIndex,
			      int srcRow,
			      int count,
			      const QModelIndex& dstParentIndex,
			      int dstRow) override;

		QStringList mimeTypes() const override;
		bool dropMimeData(const QMimeData* data,
				  Qt::DropAction action,
				  int row,
				  int column,
				  const QModelIndex& parent) override;
		QMimeData* mimeData(const QModelIndexList& indexes) const override;
		bool canDropMimeData(const QMimeData* data,
				     Qt::DropAction action,
				     int row,
				     int column,
				     const QModelIndex& parent) const override;

		GUITreeItem* getRoot();
		static GUITreeModel* build(QObject* o);
		static void buildItem(QObject* o, GUITreeItem* parent);
		static void refreshChildren(GUITreeItem* item);

		void setReparenter(WidgetReparenter* rp);
		void setInserter(WidgetInserter* i);
		void setDeleter(WidgetDeleter* d);

	protected:
		GUITreeItem* rootItem;
		WidgetReparenter* reparenter;
		WidgetInserter* inserter;
		WidgetDeleter* deleter;

		static void buildItemChildren(QObject* o, GUITreeItem* thisItem);

		bool dropMoveWidget(const QMimeData* data,
				    Qt::DropAction action,
				    int row,
				    int column,
				    const QModelIndex& parent);

		bool dropWidgetType(const QMimeData* data,
				    Qt::DropAction action,
				    int row,
				    int column,
				    const QModelIndex& parent_idx);
};

#endif
