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

#include "TabWidgetCreator.h"
#include "../widgets/ManagedWidget.h"
#include <QTabWidget>

/* Helper class for traversal of Aftermath GUI */
class ManagedTabWidget : public ManagedContainerWidget, public QTabWidget
{
	public:
		virtual void unparent() {
			this->setParent(NULL);
		}

		virtual size_t getNumChildren() {
			return this->count();
		}

		virtual void addChild(QWidget* w, size_t idx) {
			this->insertTab(idx, w, "");
		}

		virtual void removeChild(QWidget* w) {
			w->setParent(NULL);
		}

		virtual const char* getName() {
			return "amgui_tabs";
		}

		virtual QObject* getQObject() {
			return this;
		}

		virtual QObject* getNthChild(size_t n) {
			return this->widget(n);
		}
};

TabWidgetCreator::TabWidgetCreator() :
	ContainerWidgetCreator("amgui_tabs")
{
}

QWidget* TabWidgetCreator::instantiate(
	const struct am_object_notation_node_group* n)
{
	return new ManagedTabWidget();
}

void TabWidgetCreator::addChildren(const struct am_object_notation_node_group* n,
				   QWidget* parent,
				   std::list<QWidget*>& children)
{
	QTabWidget* tw = dynamic_cast<QTabWidget*>(parent);
	size_t i = 0;
	size_t num_tabs = children.size();
	struct am_object_notation_node_string* s;
	struct am_object_notation_node* nl;
	struct am_object_notation_node_list* l;

	for(auto child: children)
		tw->addTab(child, "");

	if(!(nl = am_object_notation_node_group_get_member_def(n, "tab_names")))
		return;

	if(nl->type != AM_OBJECT_NOTATION_NODE_TYPE_LIST) {
		throw WidgetCreator::Exception(
			"List of tab names is not a list.");
	}

	l = (typeof(l))nl;

	if(!am_object_notation_is_string_list(l)) {
		throw WidgetCreator::Exception(
			"List of tab names is not a list of strings.");
	}

	am_object_notation_for_each_list_item_string(l, s) {
		if(i < num_tabs)
			tw->setTabText(i, s->value);
		else
			break;

		i++;
	}
}
