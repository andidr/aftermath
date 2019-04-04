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

#include "TelamonCandidateTreeWidgetCreator.h"
#include "../widgets/TelamonCandidateTreeWidget.h"
#include "../widgets/ManagedWidget.h"
#include "../../dfg/nodes/gui/telamon_candidate_tree.h"

/* Helper class for traversal of Aftermath GUI */
AM_ALIAS_WIDGET(ManagedTelamonCandidateTreeWidget,
		TelamonCandidateTreeWidget,
		"amgui_telamon_candidate_tree")

TelamonCandidateTreeWidgetCreator::TelamonCandidateTreeWidgetCreator() :
	NonContainerWidgetCreator("amgui_telamon_candidate_tree")
{
}

QWidget* TelamonCandidateTreeWidgetCreator::instantiateDefault()
{
	return new ManagedTelamonCandidateTreeWidget();
}

QWidget* TelamonCandidateTreeWidgetCreator::instantiate(
	const struct am_object_notation_node_group* n)
{
	return this->instantiateDefault();
}

const std::string TelamonCandidateTreeWidgetCreator::getDFGNodeTypeName()
{
	return "am::gui::telamon::candidate_tree";
}

void TelamonCandidateTreeWidgetCreator::associateDFGNode(QWidget* w, struct am_dfg_node* n)
{
	TelamonCandidateTreeWidget* t;
	struct am_dfg_amgui_telamon_candidate_tree_node* tn;

	t = static_cast<TelamonCandidateTreeWidget*>(w);
	tn = reinterpret_cast<struct am_dfg_amgui_telamon_candidate_tree_node*>(n);
	t->setDFGNode(n);
	tn->tree = t;
}
