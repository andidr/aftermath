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
#include "../widgets/ManagedWidget.h"
#include <QSplitter>

/* Helper class for traversal of Aftermath GUI */
AM_RESTRICTED_TEMPLATED_ALIAS_CONTAINER_WIDGET(
	ManagedSplitterWidgetBase, QSplitter, 2)

template<typename NAME_T>
class ManagedSplitterWidget : public ManagedSplitterWidgetBase<NAME_T> {
	public:
		virtual QObject* getNthChild(size_t n) {
			return this->widget(n);
		}
};

/* Modified QSplitter that applies an initially set list of stretch values every
 * time a widget is added to the splitter. This way, the stretch values can be
 * set before the splitter has receoived its final set of children. */
template<typename NAME_T>
class SplitterWithInitialStretch : public ManagedSplitterWidget<NAME_T> {
	public:
		SplitterWithInitialStretch() : ManagedSplitterWidget<NAME_T>()
		{ }

		void setStretch(const QList<int>& stretch)
		{
			this->stretch = stretch;
		}

		void addWidget(QWidget* w)
		{
			QSplitter::addWidget(w);
			this->setSizes(this->stretch);
		}

	protected:
		QList<int> stretch;
};

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

		QWidget* instantiateDefault()
		{
			SplitterWithInitialStretch<T>* s;
			QList<int> stretchFactors;

			s = new SplitterWithInitialStretch<T>();

			s->setSizePolicy(QSizePolicy::Expanding,
					 QSizePolicy::Expanding);
			s->setOrientation(orientation);

			return s;
		}

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			struct am_object_notation_node* nstretch;
			struct am_object_notation_node_list* lstretch;
			struct am_object_notation_node_uint64* iter;
			SplitterWithInitialStretch<T>* s;
			QList<int> stretchFactors;

			/* Process stretch factors */
			nstretch = am_object_notation_node_group_get_member_def(
				n, "stretch");

			if(nstretch) {
				lstretch = (typeof(lstretch))nstretch;

				if(!am_object_notation_is_uint64_list(lstretch))
					throw Exception("Member stretch must be "
							"a list of integers");

				am_object_notation_for_each_list_item_uint64(
					lstretch, iter)
				{
					/* Scale values in order to force ratios
					 * rather than sizes in pixels. */
					stretchFactors.append(iter->value *
							      100000);

					if(iter->value < 1)
						throw Exception("Stretch value "
								"must be "
								"greater than "
								"zero");
				}
			}

			s = new SplitterWithInitialStretch<T>();

			s->setSizePolicy(QSizePolicy::Expanding,
					 QSizePolicy::Expanding);
			s->setOrientation(orientation);
			s->setStretch(stretchFactors);

			return s;
		}

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children)
		{
			SplitterWithInitialStretch<T>* s;

			s = static_cast<SplitterWithInitialStretch<T>*>(parent);

			for(auto child: children)
				s->addWidget(child);
		}
};

/* Widget creator creating horizontal splitters. The expected node format is
 *
 *   amgui_hsplitter {
 *     @optional stretch: [...],
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(HSplitterStrType, "amgui_hsplitter")
using HSplitterWidgetCreator =
	SplitterWidgetCreator<Qt::Horizontal, HSplitterStrType>;

/* Widget creator creating vertical splitters. The expected node format is
 *
 *   amgui_vsplitter {
 *     @optional stretch: [...],
 *     children: [ ... ]
 *   }
 */
AM_CXX_MKSTRCONSTTYPE(VSplitterStrType, "amgui_vsplitter")
using VSplitterWidgetCreator =
	SplitterWidgetCreator<Qt::Vertical, VSplitterStrType>;

#endif
