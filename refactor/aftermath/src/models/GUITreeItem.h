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

#ifndef AM_GUITREEITEM_H
#define AM_GUITREEITEM_H

#include "../Exception.h"
#include "../gui/widgets/ManagedWidget.h"
#include <cstdlib>
#include <QList>
#include <string>

/* A single item of a tree of widgets. It's children may be direct children or
 * indirect descendants on a path that contains no managed widgets (e.g., if the
 * widget associated to the GUITreeItem has a non-managed QObject child in the
 * QT object hierarchy that in turn has a child that is a ManagedWidget. */
class GUITreeItem {
	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg) { };
		};

		explicit GUITreeItem(ManagedWidget* w,
				     GUITreeItem* parent = NULL);
		~GUITreeItem();

		void addChild(GUITreeItem* child);
		void insertChild(GUITreeItem* child, int idx);
		GUITreeItem* getChild(size_t idx);
		size_t getNumChildren() const;
		ManagedWidget* getWidget();
		GUITreeItem* getParent();
		void setParent(GUITreeItem* p);
		int getChildIndex();
		bool removeChildren(int index, int count);
		bool deleteChildren(int index, int count);
		void destroyChildren();

	private:
		QList<GUITreeItem*> children;
		ManagedWidget* widget;
		GUITreeItem* parent;
};

#endif
