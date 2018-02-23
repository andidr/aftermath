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

#ifndef AM_TOOLBARWIDGETCREATOR_H
#define AM_TOOLBARWIDGETCREATOR_H

#include "GUIFactory.h"
#include "../../cxx_extras.h"
#include <QToolBar>

/* Generic creator for toolbar widgets */
template<enum Qt::Orientation orientation, typename T,
	 QSizePolicy::Policy HP, QSizePolicy::Policy VP>
class ToolbarWidgetCreator : public ContainerWidgetCreator {
	public:
		class Exception : public WidgetCreator::Exception {
			public:
				Exception(const std::string& msg) :
					WidgetCreator::Exception(msg) { };
		};

		ToolbarWidgetCreator() :
			ContainerWidgetCreator(T::strconst())
		{ }

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			QToolBar* t = new QToolBar();

			t->setSizePolicy(HP, VP);
			t->setOrientation(orientation);

			return t;
		}

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children)
		{
			QToolBar* t;

			t = static_cast<QToolBar*>(parent);

			for(auto child: children)
				t->addWidget(child);
		}
};

/* Widget creator creating horizontal toolbars. The expected node format is
 *
 *   amgui_htoolbar {
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(HToolbarStrType, "amgui_htoolbar")
using HToolbarWidgetCreator =
	ToolbarWidgetCreator<Qt::Horizontal, HToolbarStrType,
			     QSizePolicy::Expanding, QSizePolicy::Minimum>;

/* Widget creator creating vertical toolbars. The expected node format is
 *
 *   amgui_vtoolbar {
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(VToolbarStrType, "amgui_vtoolbar")
using VToolbarWidgetCreator =
	ToolbarWidgetCreator<Qt::Vertical, VToolbarStrType,
			     QSizePolicy::Minimum, QSizePolicy::Expanding>;

#endif
