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

#include "GUITreeItem.h"

GUITreeItem::GUITreeItem(ManagedWidget* w, GUITreeItem* parent)
	: widget(w), parent(parent)
{ }

GUITreeItem::~GUITreeItem()
{
	this->destroyChildren();
}

/* Inserts a child at the specified index */
void GUITreeItem::insertChild(GUITreeItem* child, int idx)
{
	this->children.insert(idx, child);
	child->setParent(this);
}

/* Appends a child at the end of the list of children */
void GUITreeItem::addChild(GUITreeItem* child)
{
	this->children.append(child);
	child->setParent(this);
}

/* Returns the child at the zero-based position idx. Throws an exception if the
 * index is invalid. */
GUITreeItem* GUITreeItem::getChild(size_t idx)
{
	if(idx >= this->getNumChildren())
		throw Exception("Invalid child index");

	return this->children.value(idx);
}

/* Returns the number of children of the item */
size_t GUITreeItem::getNumChildren() const
{
	return this->children.count();
}

/* Returns the widget associated to the item */
ManagedWidget* GUITreeItem::getWidget()
{
	return this->widget;
}

/* Sets the parent of the item */
void GUITreeItem::setParent(GUITreeItem* p)
{
	this->parent = p;
}

/* Returns the parent of this item or NULL if it does not have a parent */
GUITreeItem* GUITreeItem::getParent()
{
	return this->parent;
}

/* Returns the index of this GUITreeItem in the list of children of its
 * parent. If it doesn't have a parent, an exception is thrown. */
int GUITreeItem::getChildIndex()
{
	if(this->parent)
		return this->parent->children.indexOf(this);
	else
		throw Exception("GUITreeItem does not have a parent");

	return 0;
}

/* Removes and deletes the child at the given index and the count following
 * children. Returns true if the children could be removed, otherwise false.
 */
bool GUITreeItem::deleteChildren(int index, int count)
{
	int end_index = index + count - 1;

	if(index < 0 || end_index >= this->children.count())
		return false;

	for(int i = 0; i < count; i++)
		delete this->children.takeAt(index);

	return true;
}

/* Removes the child at the given index and the count following
 * children. Returns true if the children could be removed, otherwise false.
 */
bool GUITreeItem::removeChildren(int index, int count)
{
	int end_index = index + count - 1;

	if(index < 0 || end_index >= this->children.count())
		return false;

	for(int i = 0; i < count; i++)
		this->children.removeAt(index);

	return true;
}

/* Deletes and removes all children */
void GUITreeItem::destroyChildren()
{
	qDeleteAll(this->children);
	this->children.clear();
}
