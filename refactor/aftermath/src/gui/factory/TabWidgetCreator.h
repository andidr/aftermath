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

#ifndef AM_TABWIDGETCREATOR_H
#define AM_TABWIDGETCREATOR_H

#include "GUIFactory.h"

/* Widget creator creating tab widgets. The expected node format is:
 *
 *   amgui_tabs {
 *      tab_names: ["...", ...]
 *      children: [ ... ]
 *   }
 *
 * tab_names should contain one string per child, indicating the label to be
 * used for the child's tab. If the number of names is smaller than the number
 * of tabs, unlabeled tabs will receive an empty label. Excess names are
 * ignored.
 */
class TabWidgetCreator : public ContainerWidgetCreator {
	public:
		TabWidgetCreator();

		QWidget*
		instantiate(const struct am_object_notation_node_group* n);

		QWidget* instantiateDefault();

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children);
};

#endif
