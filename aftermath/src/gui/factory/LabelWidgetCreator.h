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

#ifndef AM_LABELWIDGETCREATOR_H
#define AM_LABELWIDGETCREATOR_H

#include "GUIFactory.h"

/* Widget creator creating label widgets. The expected node format is:
 *
 *   amgui_label {
 *      text: "..."
 *   }
 *
 * Where the text is optional.
 */
class LabelWidgetCreator : public NonContainerWidgetCreator {
	public:
		LabelWidgetCreator();

		QWidget* instantiateDefault();

		QWidget*
		instantiate(const struct am_object_notation_node_group* n);

		const std::string getDFGNodeTypeName();
		void associateDFGNode(QWidget* w, struct am_dfg_node* n);
};

#endif
