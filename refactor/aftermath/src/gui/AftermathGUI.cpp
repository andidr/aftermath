/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "AftermathGUI.h"
#include <QWidget>

AftermathGUI::AftermathGUI() :
	root(NULL)
{
}

AftermathGUI::~AftermathGUI()
{
	/* Delete all widgets without a parent (if a widget has a parent, QT
	 * takes care of deletion through the parent's destructor ) */
	for(auto w: this->widgets)
		if(this->isWidgetUnbound(w.second))
			delete w.second;
}

/**
 * Adds a widget to the list of widgets and associates it with the ID id.
 */
void AftermathGUI::addWidget(QWidget* w, const std::string& id)
{
	auto res = this->widgets.insert(std::make_pair(id, w));

	if(!res.second)
		throw DuplicateIDException(id);

	try {
		this->revWidgets.insert(std::make_pair(w, id));
	} catch(...) {
		this->widgets.erase(id);
		throw;
	}
}

/**
 * Removes a widget by ID
 */
void AftermathGUI::removeWidget(const std::string& id)
{
	QWidget* w = this->getWidget(id);

	this->widgets.erase(id);
	this->revWidgets.erase(w);
	this->markWidgetBound(w);
}

/**
 * Removes a widget by widget pointer
 */
void AftermathGUI::removeWidget(QWidget* w)
{
	const std::string& id = this->getID(w);

	this->widgets.erase(id);
	this->revWidgets.erase(w);
}

/**
 * Removes a widget by widget pointer. If the widget does not have a parent, its
 * destructor is called.
 */
void AftermathGUI::removeWidgetCheckDestroy(QWidget* w)
{
	const std::string& id = this->getID(w);

	auto it = this->widgets.find(id);
	auto revit = this->revWidgets.find(w);

	/* Safe to erase now, since erase() called with an iterator does not
	 * throw an exception */
	this->widgets.erase(it);
	this->revWidgets.erase(revit);

	if(this->isWidgetUnbound(w))
		delete w;

	this->markWidgetBound(w);
}

/**
 * Returns the widget by ID. If no such widget exists, an exception is thrown.
 */
QWidget* AftermathGUI::getWidget(const std::string& id)
{
	auto it = this->widgets.find(id);

	if(it == this->widgets.end())
		throw NonexistentIDException(id);

	return it->second;
}

/**
 * Returns the ID of a widget w. If no such widget has been registered, an
 * exception is thrown.
 */
const std::string& AftermathGUI::getID(QWidget* w)
{
	auto it = this->revWidgets.find(w);

	if(it == this->revWidgets.end())
		throw NonexistentWidgetException();

	return it->second;
}

/**
 * Returns the root widget or NULL if no root widget has been set.
 */
QWidget* AftermathGUI::getRootWidget() noexcept
{
	return this->root;
}

/* Sets the root widget. The widget must have been added to the list of widgets
 * previously. */
void AftermathGUI::setRootWidget(QWidget* w)
{
	if(this->revWidgets.find(w) == this->revWidgets.end())
		throw NonexistentWidgetException();

	this->root = w;
}

/* Same as setRootWidget(), but omits check for existence of w in the list of
 * widgets and thus never throws an exception. */
void AftermathGUI::setRootWidgetNE(QWidget* w) noexcept
{
	this->root = w;
}

/* Mark widget as bound, such that it does not get destroyed upon destruction of
 * the AftermathGUI */
void AftermathGUI::markWidgetBound(QWidget* w) noexcept
{
	auto it = this->unboundWidgets.find(w);

	if(it != this->unboundWidgets.end())
		this->unboundWidgets.erase(it);
}

/* Mark widget as unbound, such that it gets destroyed upon destruction of the
 * AftermathGUI */
void AftermathGUI::markWidgetUnbound(QWidget* w)
{
	auto it = this->unboundWidgets.find(w);

	if(it == this->unboundWidgets.end())
		this->unboundWidgets.insert(w);
}

/* Checks if a widget has been marked as unbound.
 */
bool AftermathGUI::isWidgetUnbound(QWidget* w) noexcept
{
	auto it = this->unboundWidgets.find(w);

	return (it != this->unboundWidgets.end());
}
