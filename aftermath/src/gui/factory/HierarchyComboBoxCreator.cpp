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

#include "HierarchyComboBoxCreator.h"
#include "../widgets/ManagedWidget.h"
#include "../../dfg/nodes/gui/hierarchy_combobox.h"

/* Helper class for traversal of Aftermath GUI */
AM_ALIAS_WIDGET(ManagedHierarchyComboBox,
		HierarchyComboBox,
		"amgui_hierarchy_combobox")

HierarchyComboBoxCreator::HierarchyComboBoxCreator() :
	NonContainerWidgetCreator("amgui_hierarchy_combobox")
{
}

QWidget* HierarchyComboBoxCreator::instantiateDefault()
{
	return new ManagedHierarchyComboBox();
}

QWidget* HierarchyComboBoxCreator::instantiate(
	const struct am_object_notation_node_group* n)
{
	return this->instantiateDefault();
}

const std::string HierarchyComboBoxCreator::getDFGNodeTypeName()
{
	return "am::gui::hierarchy_combobox";
}

void HierarchyComboBoxCreator::associateDFGNode(QWidget* w,
						struct am_dfg_node* n)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hn;
	HierarchyComboBox* c;

	c = static_cast<HierarchyComboBox*>(w);
	hn = reinterpret_cast<struct am_dfg_amgui_hierarchy_combobox_node*>(n);

	c->setDFGNode(n);
	hn->widget = c;
}
