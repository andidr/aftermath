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

#include "GUIFactory.h"
#include "../../cxx_extras.h"
#include <cstdint>
#include <cinttypes>
#include <cstring>

WidgetCreator::WidgetCreator(const std::string& groupName) :
	groupName(groupName)
{
}

GUIFactory::GUIFactory() :
	currID(0)
{
	this->addCreator(&this->rootCreator);
}

GUIFactory::~GUIFactory()
{
	for(WidgetCreator* c: this->creators)
		if(c != &this->rootCreator)
			delete c;
}

/**
 * Generates a new ID prefixed by the given prefix.
 */
std::string GUIFactory::generateID(const std::string& prefix)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%" PRIu64, this->currID++);
	return prefix + buf;
}

/**
 * Adds a creator to the GUI factory. Ownership is transferred to the factory
 * and the creator is destroyed at the end of the factory's lifetime.
 */
void GUIFactory::addCreator(WidgetCreator* c)
{
	this->creators.insert(c);
}

/**
 * Returns the creator for a given group name or NULL if no such creator has
 * been registered.
 */
WidgetCreator* GUIFactory::findCreator(const std::string& groupName)
{
	auto it = this->creators.find(groupName);

	if(it != this->creators.end())
		return *it;

	return NULL;
}

/**
 * Builds all the children of a widget according to the list of group nodes
 * children and then adds the children to the parent.
 */
void GUIFactory::buildChildren(std::list<WidgetMetaInfo>& widgets,
			       WidgetCreator* creator,
			       const struct am_object_notation_node_group* n,
			       QWidget* parent,
			       struct am_object_notation_node* children)
{
	struct am_object_notation_node_list* lchildren;
	struct am_object_notation_node_group* gchild;
	std::list<QWidget*> childList;
	QWidget* child;

	if(!children)
		return;

	if(children->type != AM_OBJECT_NOTATION_NODE_TYPE_LIST) {
		throw ObjectNotationFileException(
			"Member 'children' is not a list.");
	}

	lchildren = (typeof(lchildren))children;

	if(!am_object_notation_is_group_list(lchildren)) {
		throw ObjectNotationFileException(
			"Member 'children' is not a list of group nodes.");
	}

	am_object_notation_for_each_list_item_group(lchildren, gchild) {
		child = this->buildWidget(widgets, gchild);
		childList.push_back(child);
	}

	creator->addChildren(n, parent, childList);
}

/**
 * Instantiate a widget based on an object notation group node. Throws a
 * NoCreatorFoundException exception if no matching creator could be found for
 * the node.
 */
QWidget* GUIFactory::buildWidget(std::list<WidgetMetaInfo>& widgets,
				 const struct am_object_notation_node_group* n)
{
	WidgetCreator* creator;
	QWidget* rawWidget;
	struct am_object_notation_node* children;
	const char* cid;
	std::string id;

	if(!(creator = this->findCreator(n->name)))
		throw NoCreatorFoundException(n->name);

	if(am_object_notation_eval_retrieve_string(&n->node, "id", &cid))
		id = this->generateID(n->name);
	else
		id = cid;

	rawWidget = creator->instantiate(n);

	try {
		widgets.push_back(WidgetMetaInfo(rawWidget, id));
	} catch(...) {
		delete rawWidget;
		throw;
	}

	if(creator->isContainer()) {
		children = am_object_notation_node_group_get_member_def(
			n, "children");

		this->buildChildren(widgets, creator, n, rawWidget, children);
	}

	return rawWidget;
}

/**
 * Registers a list of created widgets atomically at the GUI gui, i.e., either
 * all widgets are added or none. On success, Ownership for all widgets is
 * transferred to the GUI and the list is emptied.
 */
void GUIFactory::commitWidgets(AftermathGUI& gui,
			       std::list<WidgetMetaInfo>& widgets)
{
	std::list<WidgetMetaInfo>::iterator curr;

	for(curr = widgets.begin(); curr != widgets.end(); ++curr) {
		try {
			gui.addWidget(curr->getWidget(), curr->getID());
		} catch(...) {
			for(auto it = widgets.begin(); it != curr; ++it)
				gui.removeWidget(it->getID());

			throw;
		}
	}

	/* Mark widgets without parent as unbound */
	for(curr = widgets.begin(); curr != widgets.end(); ++curr) {
		try {
			if(!curr->getWidget()->parent())
				gui.markWidgetUnbound(curr->getWidget());
		} catch(...) {
			/* On error: rollback addition of widgets marked as
			 * unbound to prevent their destruction */
			for(auto it = widgets.begin(); it != curr; ++it)
				if(!curr->getWidget()->parent())
					gui.markWidgetBound(it->getWidget());

			/* Remove all widgets from the GUI*/
			for(auto mi: widgets)
				gui.removeWidget(mi.getWidget());

			throw;
		}
	}

	widgets.clear();
}

/**
 * Builds an entire GUI from the root object notation node root. Returns the
 * GUI's toplevel widget.
 */
void GUIFactory::buildGUI(AftermathGUI* gui,
			  struct am_object_notation_node* root)
{
	QWidget* rootWidget = NULL;
	struct am_object_notation_node_group* groot;
	std::list<WidgetMetaInfo> widgets;

	if(root->type != AM_OBJECT_NOTATION_NODE_TYPE_GROUP) {
		throw ObjectNotationFileException(
			"Root node is not a group node");
	}

	groot = (typeof(groot))root;

	if(strcmp(groot->name, "am_gui") != 0) {
		throw ObjectNotationFileException(
			"Root group node is not am_gui.");
	}

	try {
		rootWidget = this->buildWidget(widgets, groot);
		this->commitWidgets(*gui, widgets);
		gui->setRootWidget(rootWidget);
	} catch(...) {
		/* Unparent all widgets in order to prevent QT from recursive
		 * invocation of destructors */
		for(auto& mi: widgets)
			if(mi.getWidget())
				mi.setWidget(NULL);

		for(auto& mi: widgets)
			if(mi.getWidget())
				delete mi.getWidget();

		throw;
	}
}

/**
 * Builds a GUI from an object notation file. Returns the root widget of the
 * GUI.
 */
void GUIFactory::buildGUI(AftermathGUI* gui, const char* filename)
{
	struct am_object_notation_node* root;

	if(!(root = am_object_notation_load(filename))) {
		throw ObjectNotationFileException(
			std::string("Could not open object notation file '") +
			filename);
	}

	try {
		this->buildGUI(gui, root);
	} catch(...) {
		am_object_notation_node_destroy(root);
		free(root);
		throw;
	}

	am_object_notation_node_destroy(root);
	free(root);
}
