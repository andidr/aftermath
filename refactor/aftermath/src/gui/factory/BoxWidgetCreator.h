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

#ifndef AM_BOXWIDGETCREATOR_H
#define AM_BOXWIDGETCREATOR_H

#include "GUIFactory.h"
#include "../../cxx_extras.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

/* Generic creator for box widgets */
template<typename L, typename T, Qt::AlignmentFlag A>
class BoxWidgetCreator : public LayoutContainerWidgetCreator {
	public:
		class Exception : public WidgetCreator::Exception {
			public:
				Exception(const std::string& msg) :
					WidgetCreator::Exception(msg) { };
		};

		BoxWidgetCreator() :
			LayoutContainerWidgetCreator(T::strconst())
		{ }

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			QWidget* w = new QWidget();

			w->setSizePolicy(QSizePolicy::Expanding,
					 QSizePolicy::Expanding);

			try {
				w->setLayout(new L());
				w->layout()->setSpacing(0);
				w->layout()->setAlignment(A);
			} catch(...) {
				delete w;
				throw;
			}

			return w;
		}
};

/* Widget creator creating horizontal boxes. The expected node format is
 *
 *   amgui_hbox {
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(HBoxStrType, "amgui_hbox")
using HBoxWidgetCreator = BoxWidgetCreator<QHBoxLayout, HBoxStrType, Qt::AlignLeft>;

/* Widget creator creating vertical boxes. The expected node format is
 *
 *   amgui_vbox {
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(VBoxStrType, "amgui_vbox")
using VBoxWidgetCreator = BoxWidgetCreator<QVBoxLayout, VBoxStrType, Qt::AlignTop>;

#endif
