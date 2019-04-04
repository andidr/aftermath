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

#include "LabelWidgetCreator.h"
#include "../widgets/ManagedWidget.h"
#include "../widgets/LabelWithDFGNode.h"
#include "../../dfg/nodes/gui/label.h"

/* Helper class for traversal of Aftermath GUI */
AM_ALIAS_WIDGET(ManagedLabel, LabelWithDFGNode, "amgui_label")

LabelWidgetCreator::LabelWidgetCreator() :
	NonContainerWidgetCreator("amgui_label")
{
}

QWidget* LabelWidgetCreator::instantiateDefault()
{
	QLabel* l = new ManagedLabel();

	l->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

	return l;
}

QWidget* LabelWidgetCreator::instantiate(const struct am_object_notation_node_group* n)
{
	const char* text;
	QLabel* l = new ManagedLabel();

	l->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

	try {
		if(am_object_notation_eval_retrieve_string(&n->node,
							   "text",
							   &text) == 0)
		{
			l->setText(text);
		}
	} catch(...) {
		delete l;
		throw;
	}

	return l;
}

const std::string LabelWidgetCreator::getDFGNodeTypeName()
{
	return "am::gui::label";
}

void LabelWidgetCreator::associateDFGNode(QWidget* w, struct am_dfg_node* n)
{
	struct am_dfg_amgui_label_node* ln;
	ManagedLabel* l;

	l = static_cast<ManagedLabel*>(w);
	ln = reinterpret_cast<struct am_dfg_amgui_label_node*>(n);

	l->setDFGNode(n);
	ln->label = l;
}
