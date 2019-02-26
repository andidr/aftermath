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

#ifndef AM_TOOLBARBUTTONCREATOR_H
#define AM_TOOLBARBUTTONCREATOR_H

#include "GUIFactory.h"
#include "../widgets/ManagedWidget.h"
#include "../widgets/ToolbarButton.h"

/* Helper class for traversal of Aftermath GUI */
AM_ALIAS_WIDGET(ManagedToolbarToggleButton,
		ToolbarToggleButton,
		"amgui_toolbar_button")

/* Helper class for traversal of Aftermath GUI */
AM_ALIAS_WIDGET(ManagedToolbarButton,
		ToolbarButton,
		"amgui_toolbar_button")

/**
 * Virtual class implementing both toolbar push buttons (TOGGLE = false) and
 * toolbar toggle buttons (TOGGLE = true)
 */
template<bool TOGGLE>
class AbstractToolbarButtonCreator : public NonContainerWidgetCreator {
	public:
		AbstractToolbarButtonCreator(const std::string& groupName) :
			NonContainerWidgetCreator(groupName)
		{ }

		QWidget* instantiateDefault() {
			QToolButton* b;

			if(TOGGLE)
				b = new ManagedToolbarToggleButton();
			else
				b = new ManagedToolbarButton();

			b->setCheckable(TOGGLE);

			return b;
		}

		QWidget* instantiate(const struct am_object_notation_node_group* n)
		{
			const char* str;
			QToolButton* b;
			uint64_t checked;

			b = reinterpret_cast<QToolButton*>
				(this->instantiateDefault());

			try {
				if(am_object_notation_eval_retrieve_string(
					   &n->node, "text", &str) == 0)
				{
					b->setText(str);
				}

				if(am_object_notation_eval_retrieve_string(
					   &n->node, "tooltip", &str) == 0)
				{
					b->setToolTip(str);
				}

				if(am_object_notation_eval_retrieve_string(
					   &n->node, "icon", &str) == 0)
				{
					b->setIcon(QIcon(str));
				}

				if(TOGGLE) {
					if(am_object_notation_eval_retrieve_uint64(
						   &n->node, "checked", &checked) == 0)
					{
						b->setChecked(checked);
					}
				}
			} catch(...) {
				delete b;
				throw;
			}

			return b;
		}
};

/* Widget creator creating toolbar pushbutton widgets. The expected node format
 * is:
 *
 *   amgui_toolbar_button {
 *      @optional text: "...",
 *      @optional tooltip: "...",
 *      @optional icon: "..."
 *   }
 *
 */
class ToolbarButtonCreator : public AbstractToolbarButtonCreator<false> {
	public:
		ToolbarButtonCreator() :
			AbstractToolbarButtonCreator("amgui_toolbar_button")
		{ }
};

/* Widget creator creating toolbar toggle button widgets. The expected node
 * format is:
 *
 *   amgui_toolbar_togglebutton {
 *      @optional text: "...",
 *      @optional tooltip: "...",
 *      @optional icon: "..."
 *   }
 *
 */
class ToolbarToggleButtonCreator : public AbstractToolbarButtonCreator<true> {
	public:
		ToolbarToggleButtonCreator() :
			AbstractToolbarButtonCreator("amgui_toolbar_togglebutton")
		{ }
};

#endif
