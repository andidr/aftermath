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

#ifndef AM_STATUSBARWIDGETCREATOR_H
#define AM_STATUSBARWIDGETCREATOR_H

#include "GUIFactory.h"
#include <QStatusBar>

/* Widget creator creating statusbar widgets. The expected node format is:
 *
 *   amgui_statusbar {
 *     children: [ ... ]
 *   }
 *
 * Where the text is optional.
 */
class StatusbarWidgetCreator : public ContainerWidgetCreator {
	public:
		StatusbarWidgetCreator() : ContainerWidgetCreator("amgui_statusbar")
			{ }

		virtual ~StatusbarWidgetCreator() = default;

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			QStatusBar* sb = new QStatusBar();

			sb->setSizePolicy(QSizePolicy::Expanding,
					  QSizePolicy::Minimum);

			return sb;
		}

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children)
		{
			QStatusBar* sb = dynamic_cast<QStatusBar*>(parent);

			for(auto child: children)
				sb->addWidget(child, 1);
		}
};

#endif
