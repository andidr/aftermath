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
#include "../widgets/ManagedWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

/* Helper class for traversal of Aftermath GUI */
AM_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET(ManagedBoxWidget, QWidget, QBoxLayout)

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

		QWidget* instantiateDefault() {
			QWidget* w;
			int margins[4] = { 5, 5, 5, 5 };

			w = new ManagedBoxWidget<T>();
			w->setSizePolicy(QSizePolicy::Expanding,
					 QSizePolicy::Expanding);

			try {
				w->setLayout(new L());
				w->layout()->setSpacing(0);
				w->layout()->setAlignment(A);
				w->layout()->setContentsMargins(margins[0],
								margins[1],
								margins[2],
								margins[3]);
			} catch(...) {
				delete w;
				throw;
			}

			return w;
		}

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			QWidget* w;
			struct am_object_notation_node* nmargin;
			struct am_object_notation_node_uint64* imargin;
			int margins[4] = { 5, 5, 5, 5 };

			const char* margin_names[4] = {
				"margin_left", "margin_top",
				"margin_right", "margin_bottom"
			};

			for(size_t i = 0; i < 4; i++) {
				nmargin = am_object_notation_node_group_get_member_def(
					n, margin_names[i]);

				if(!nmargin)
					continue;

				if(nmargin->type !=
				   AM_OBJECT_NOTATION_NODE_TYPE_UINT64)
				{
					throw Exception(std::string("Member ") +
							margin_names[i] +
							" must be an integer");
				}

				imargin = (typeof(imargin))nmargin;
				margins[i] = imargin->value;
			}

			w = this->instantiateDefault();

			try {
				w->layout()->setContentsMargins(margins[0],
								margins[1],
								margins[2],
								margins[3]);
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
 *     @optional margin_top: ...,
 *     @optional margin_left: ...,
 *     @optional margin_bottom: ...,
 *     @optional margin_right: ...,
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(HBoxStrType, "amgui_hbox")
using HBoxWidgetCreator = BoxWidgetCreator<QHBoxLayout, HBoxStrType, Qt::AlignLeft>;

/* Widget creator creating vertical boxes. The expected node format is
 *
 *   amgui_vbox {
 *     @optional margin_top: ...,
 *     @optional margin_left: ...,
 *     @optional margin_bottom: ...,
 *     @optional margin_right: ...,
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(VBoxStrType, "amgui_vbox")
using VBoxWidgetCreator = BoxWidgetCreator<QVBoxLayout, VBoxStrType, Qt::AlignTop>;

#endif
