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

#include "ButtonWidgetCreator.h"
#include <QPushButton>

ButtonWidgetCreator::ButtonWidgetCreator() :
	NonContainerWidgetCreator("amgui_button")
{
}

QWidget* ButtonWidgetCreator::instantiate(const struct am_object_notation_node_group* n)
{
	const char* text;
	QPushButton* b = new QPushButton();

	try {
		if(am_object_notation_eval_retrieve_string(&n->node,
							   "text",
							   &text) == 0)
		{
			b->setText(text);
		}
	} catch(...) {
		delete b;
		throw;
	}

	return b;
}
