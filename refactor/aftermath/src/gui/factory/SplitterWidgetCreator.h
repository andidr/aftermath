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

#ifndef AM_SPLITTERWIDGETCREATOR_H
#define AM_SPLITTERWIDGETCREATOR_H

#include "GUIFactory.h"
#include "../../cxx_extras.h"
#include <QSplitter>

/* Generic creator for splitter widgets */
template<enum Qt::Orientation orientation, typename T>
class SplitterWidgetCreator : public ContainerWidgetCreator {
	public:
		class Exception : public WidgetCreator::Exception {
			public:
				Exception(const std::string& msg) :
					WidgetCreator::Exception(msg) { };
		};

		SplitterWidgetCreator() :
			ContainerWidgetCreator(T::strconst())
		{ }

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			QSplitter* s = new QSplitter();

			s->setSizePolicy(QSizePolicy::Expanding,
					 QSizePolicy::Expanding);
			s->setOrientation(orientation);

			return s;
		}

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children)
		{
			QSplitter* s = static_cast<QSplitter*>(parent);

			for(auto child: children)
				s->addWidget(child);
		}
};

/* Widget creator creating horizontal splitters. The expected node format is
 *
 *   amgui_hsplitter {
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(HSplitterStrType, "amgui_hsplitter")
using HSplitterWidgetCreator =
	SplitterWidgetCreator<Qt::Horizontal, HSplitterStrType>;

/* Widget creator creating vertical splitters. The expected node format is
 *
 *   amgui_vsplitter {
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(VSplitterStrType, "amgui_vsplitter")
using VSplitterWidgetCreator =
	SplitterWidgetCreator<Qt::Vertical, VSplitterStrType>;

#endif
