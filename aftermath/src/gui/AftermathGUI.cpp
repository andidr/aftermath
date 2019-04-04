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

extern "C" {
	#include <aftermath/core/bits.h>
}

AftermathGUI::AftermathGUI() :
	root(NULL)
{
	am_prng_u64_seed(&this->prng, ((uintptr_t)this));
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

/* Returns true if the widget w is known to the GUI */
bool AftermathGUI::hasWidget(QWidget* w)
{
	auto it = this->revWidgets.find(w);

	if(it == this->revWidgets.end()) {
		auto uit = this->unboundWidgets.find(w);

		if(uit == this->unboundWidgets.end())
			return false;
	}

	return true;
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
	this->unboundWidgets.erase(w);
}

/**
 * Removes an object and all of its descendants
 */
void AftermathGUI::removeObjectRec(QObject* o)
{
	QWidget* w;

	for(QObject* child: o->children())
		this->removeObjectRec(child);

	if((w = dynamic_cast<QWidget*>(o))) {
		if(this->hasWidget(w))
			this->removeWidget(w);
	}
}

/**
 * Removes a widget and all of its descendants
 */
void AftermathGUI::removeWidgetRec(QWidget* w)
{
	this->removeObjectRec(w);
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

/* Returns true if the GUI contains a bound widget whose ID is id, otherwise
 * false. */
bool AftermathGUI::hasID(const std::string& id)
{
	return this->widgets.find(id) != this->widgets.end();
}

/* Generates an unused ID */
std::string AftermathGUI::generateID()
{
	uint64_t rand;
	std::string id;
	uint64_t num_widgets = this->widgets.size();
	uint64_t num_widgets_msb = am_extract_msb_u64(num_widgets);
	uint64_t bitmask;

	if(num_widgets_msb == ((uint64_t)1 << 63))
		bitmask = ~((uint64_t)0);
	else
		bitmask = (num_widgets_msb << 1) - 1;

	do {
		rand = am_prng_u64_rand(&this->prng, 0, UINT64_MAX) & bitmask;
		id = "id" + std::to_string(rand);
	} while(this->hasID(id));

	return id;
}
