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

#include "GUITreeModel.h"
#include <QtWidgets>

GUITreeModel::GUITreeModel(WidgetReparenter* reparenter,
			   WidgetInserter* inserter,
			   WidgetDeleter* deleter,
			   QObject* parent)
	: QAbstractItemModel(parent),
	  reparenter(reparenter),
	  inserter(inserter),
	  deleter(deleter)
{
	this->rootItem = new GUITreeItem(NULL);
}

GUITreeModel::~GUITreeModel()
{
	delete this->rootItem;
}

int GUITreeModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant GUITreeModel::data(const QModelIndex& index, int role) const
{
	GUITreeItem* item;
	ManagedWidget* widget;

	if(!index.isValid())
		return QVariant();

	if(role != Qt::DisplayRole)
		return QVariant();

	item = static_cast<GUITreeItem*>(index.internalPointer());

	if((widget = item->getWidget()))
		return widget->getName();
	else
		return "";
}

Qt::DropActions GUITreeModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions GUITreeModel::supportedDragActions() const
{
	return Qt::MoveAction;
}

Qt::ItemFlags GUITreeModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags;
	GUITreeItem* item;
	ManagedWidget* widget;
	ManagedContainerWidget* cw;

	if(!index.isValid())
		return 0;

	item = static_cast<GUITreeItem*>(index.internalPointer());

	if((widget = item->getWidget())) {
		flags |= Qt::ItemIsDragEnabled;

		if((cw = dynamic_cast<ManagedContainerWidget*>(widget))) {
			if(cw->getNumChildren() < cw->getMaxChildren())
				flags |= Qt::ItemIsDropEnabled;
		}
	}

	return flags | QAbstractItemModel::flags(index);
}

QVariant GUITreeModel::headerData(int section, Qt::Orientation orientation,
				  int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return "Widget";

	return QVariant();
}

QModelIndex GUITreeModel::index(
	int row, int column, const QModelIndex& parentIdx) const
{
	GUITreeItem* parent;
	GUITreeItem* child;

	if(!this->hasIndex(row, column, parentIdx))
		return QModelIndex();

	if(!parentIdx.isValid())
		parent = rootItem;
	else
		parent = static_cast<GUITreeItem*>(parentIdx.internalPointer());

	if((child = parent->getChild(row)))
		return this->createIndex(row, column, child);
	else
		return QModelIndex();
}

QModelIndex GUITreeModel::parent(const QModelIndex& index) const
{
	GUITreeItem* child;
	GUITreeItem* parent;

	if(!index.isValid())
		return QModelIndex();

	child = static_cast<GUITreeItem*>(index.internalPointer());
	parent = child->getParent();

	if(parent == this->rootItem)
		return QModelIndex();

	return this->createIndex(parent->getChildIndex(), 0, parent);
}

int GUITreeModel::rowCount(const QModelIndex& idx) const
{
	GUITreeItem* parent;

	if(idx.column() > 0)
		return 0;

	if(!idx.isValid())
		parent = this->rootItem;
	else
		parent = static_cast<GUITreeItem*>(idx.internalPointer());

	return parent->getNumChildren();
}

/* Returns the virtual root item of the model */
GUITreeItem* GUITreeModel::getRoot()
{
	return this->rootItem;
}

bool GUITreeModel::removeRows(int row, int count, const QModelIndex& parentIndex)
{

	//negative
	GUITreeItem* parentItem;
	GUITreeItem* item;
	ManagedWidget* widget;

	if(!parentIndex.isValid() || row < 0 || count != 1)
		return false;

	parentItem = static_cast<GUITreeItem*>(parentIndex.internalPointer());
	item = parentItem->getChild(row);
	widget = item->getWidget();

	this->beginRemoveRows(parentIndex, row, row + count - 1);

	if(this->deleter && widget) {
		if(!((*this->deleter)(widget))) {
			this->endRemoveRows();
			return false;
		}
	}

	parentItem->deleteChildren(row, count);
	this->endRemoveRows();

	return true;

	return false;
}

void GUITreeModel::buildItemChildren(QObject* o, GUITreeItem* thisItem)
{
	QWidget* qw;
	QWidget* qwchild;
	QLayout* l = NULL;
	QLayoutItem* li;
	ManagedContainerWidget* cw;

	/* Try to get items from aftermath container widget */
	if((cw = dynamic_cast<ManagedContainerWidget*>(o))) {
		for(size_t i = 0; i < cw->getNumChildren(); i++)
			GUITreeModel::buildItem(cw->getNthChild(i), thisItem);
	} else {
		/* Otherwise try to get items from layout */
		if((qw = dynamic_cast<QWidget*>(o)))
			l = qw->layout();

		if(l) {
			for(int i = 0; i < l->count(); i++) {
				if((li = l->itemAt(i))) {
					if((qwchild = li->widget())) {
						GUITreeModel::buildItem(
							qwchild, thisItem);
					}
				}
			}
		} else {
			/* Fallback: get from child list; however, this might
			 * not be in the correct order */
			for(QObject* child: o->children())
				GUITreeModel::buildItem(child, thisItem);
		}
	}
}

/* Builds an entire hierarchy of GUITreeItems starting from o using parent as
 * the root */
void GUITreeModel::buildItem(QObject* o, GUITreeItem* parent)
{
	ManagedWidget* aw;
	ManagedContainerWidget* cw;
	GUITreeItem* thisItem;
	GUITreeItem* nextParent = parent;

	if(!o)
		return;

	if((aw = dynamic_cast<ManagedWidget*>(o))) {
		thisItem = new GUITreeItem(aw, parent);

		try {
			parent->addChild(thisItem);
		} catch(...) {
			delete thisItem;
			throw;
		}

		if((cw = dynamic_cast<ManagedContainerWidget*>(o)))
			nextParent = thisItem;
	}

	GUITreeModel::buildItemChildren(o, nextParent);
}

/* Rebuilds the list of children of the item */
void GUITreeModel::refreshChildren(GUITreeItem* item)
{
	ManagedWidget* aw;

	item->destroyChildren();

	if((aw = item->getWidget()))
		GUITreeModel::buildItemChildren(aw->getQObject(), item);
}

/* Builds a model containing all managed widgets that are descendants of o
 * (including o if it is a managed widget) */
GUITreeModel* GUITreeModel::build(QObject* o)
{
	GUITreeModel* model = new GUITreeModel();

	try {
		GUITreeModel::buildItem(o, model->getRoot());
	} catch(...) {
		delete model;
		throw;
	}

	return model;
}

QStringList GUITreeModel::mimeTypes() const
{
	QStringList types;
	types << "application/aftermath.guitreeitem";
	types << "application/aftermath.widgetgroupname";

	return types;
}

QMimeData* GUITreeModel::mimeData(const QModelIndexList& indexes) const
{
	QMimeData* mimeData = NULL;
	GUITreeItem* item;
	static_assert(sizeof(GUITreeItem*) <= sizeof(quint64),
		      "Cannot serialize pointer in quint64");

	try {
		mimeData = new QMimeData();
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);

		for(const auto& idx: indexes) {
			if (idx.isValid()) {
				item = static_cast<GUITreeItem*>
					(idx.internalPointer());

				stream << (quint64)item;
			}
		}

		mimeData->setData("application/aftermath.guitreeitemptr",
				  encodedData);
	} catch(...) {
		delete mimeData;
		throw;
	}

	return mimeData;
}

bool GUITreeModel::canDropMimeData(const QMimeData* data,
				   Qt::DropAction action,
				   int row,
				   int column,
				   const QModelIndex& parent) const
{
	if(action == Qt::MoveAction) {
		if(data->hasFormat("application/aftermath.guitreeitemptr"))
			return true;
	} else if(action == Qt::CopyAction) {
		if(data->hasFormat("application/aftermath.widgetgroupname"))
			return true;
	}

	return false;
}

bool GUITreeModel::moveRows(const QModelIndex& srcParentIndex,
			    int srcRow,
			    int count,
			    const QModelIndex& dstParentIndex,
			    int dstRow)
{
	GUITreeItem* srcParent = static_cast<GUITreeItem*>(srcParentIndex.internalPointer());
	GUITreeItem* dstParent = static_cast<GUITreeItem*>(dstParentIndex.internalPointer());
	GUITreeItem* srcItem = srcParent->getChild(srcRow);
	ManagedContainerWidget* srcParentWidget;
	ManagedContainerWidget* dstParentWidget;
	ManagedWidget* srcWidget;

	/* Currently only supports moving a single item */
	if(count != 1)
		return false;

	if(!this->reparenter)
		return false;

	if(srcParent == dstParent && srcRow == dstRow)
		return false;

	srcWidget = srcItem->getWidget();

	if(!(srcParentWidget = dynamic_cast<ManagedContainerWidget*>(srcParent->getWidget())) ||
	   !(dstParentWidget = dynamic_cast<ManagedContainerWidget*>(dstParent->getWidget())))
	{
		return false;
	}

	if(!(*this->reparenter)(srcWidget, srcParentWidget, srcRow, dstParentWidget, dstRow))
		return false;

	//this->beginMoveRows(srcParentIndex, srcRow, srcRow, dstParentIndex, dstRow);
	emit layoutAboutToBeChanged();

	srcParent->removeChildren(srcRow, 1);

	if(srcParent == dstParent && dstRow > srcRow)
		dstRow--;

	dstParent->insertChild(srcItem, dstRow);

	//this->endMoveRows();
	emit layoutChanged();

	return true;
}

/* Handles drops of widgets (reordering / reparenting) */
bool GUITreeModel::dropMoveWidget(const QMimeData* data,
				  Qt::DropAction action,
				  int row,
				  int column,
				  const QModelIndex& newParentIndex)
{
	quint64 ptrval;
	GUITreeItem* item = NULL;
	GUITreeItem* newParentItem = NULL;
	if(!data->hasFormat("application/aftermath.guitreeitemptr"))
		return false;

	if(!this->reparenter)
		return false;

	QByteArray encodedData = data->data(
		"application/aftermath.guitreeitemptr");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	while(!stream.atEnd()) {
		/* Can only move a single item */
		if(item)
			return false;

		stream >> ptrval;
		item = (GUITreeItem*)ptrval;
	}

	QModelIndex itemIndex = this->index(item);
	QModelIndex itemParentIndex = this->parent(itemIndex);

	/* Drop directly on element -> Append at the end */
	if(row < 0) {
		newParentItem = static_cast<GUITreeItem*>(newParentIndex.internalPointer());
		row = newParentItem->getNumChildren();
	}

	return this->moveRow(itemParentIndex, itemIndex.row(), newParentIndex, row);
}

/* Handles drops of widgets (reordering / reparenting) */
bool GUITreeModel::dropWidgetType(const QMimeData* data,
				  Qt::DropAction action,
				  int row,
				  int column,
				  const QModelIndex& parent_idx)
{
	GUITreeItem* parent;
	ManagedContainerWidget* parent_widget;
	QString groupName;

	if(!data->hasFormat("application/aftermath.widgetgroupname"))
		return false;

	if(!this->inserter)
		return false;

	parent = static_cast<GUITreeItem*>(parent_idx.internalPointer());
	parent_widget = dynamic_cast<ManagedContainerWidget*>
		(parent->getWidget());

	if(!parent_widget)
		throw AftermathException("Widget is not a container widget");

	QByteArray encodedData = data->data(
		"application/aftermath.widgetgroupname");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	emit layoutAboutToBeChanged();

	while(!stream.atEnd()) {
		stream >> groupName;

		if((*this->inserter)(parent_widget, row, groupName))
			this->refreshChildren(parent);
	}

	emit layoutChanged();

	return true;
}

QModelIndex GUITreeModel::index(GUITreeItem* i)
{
	if(i->getParent()) {
		QModelIndex pidx = this->index(i->getParent());
		int cidx = i->getChildIndex();
		return this->index(cidx, 0, pidx);
	} else {
		return QModelIndex();
	}
}

bool GUITreeModel::dropMimeData(const QMimeData* data,
				Qt::DropAction action,
				int row,
				int column,
				const QModelIndex& parent_idx)
{
	if(action == Qt::MoveAction) {
		bool res = this->dropMoveWidget(data, action, row, column, parent_idx);
		return res;
	}
	else if(action == Qt::CopyAction)
		return this->dropWidgetType(data, action, row, column, parent_idx);

	return false;
}

void GUITreeModel::setReparenter(WidgetReparenter* rp)
{
	this->reparenter = rp;
}

void GUITreeModel::setInserter(WidgetInserter* i)
{
	this->inserter = i;
}

void GUITreeModel::setDeleter(WidgetDeleter* d)
{
	this->deleter = d;
}
