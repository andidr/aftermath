/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_TOOLBAR_TOGGLE_BUTTON_H
#define AM_TOOLBAR_TOGGLE_BUTTON_H

#include <QToolButton>
#include "WidgetWithDFGNode.h"

/* Non-togglable toolbar button with associated DFG node. */
class ToolbarButton : public QToolButton {
	AM_WIDGETWITHDFGNODE_DECLS
	Q_OBJECT

	public:
		ToolbarButton(QWidget* parent = NULL);
		virtual ~ToolbarButton() = default;
};

/* Togglable toolbar button with associated DFG node. */
class ToolbarToggleButton : public ToolbarButton {
	public:
		ToolbarToggleButton(QWidget* parent = NULL);
		virtual ~ToolbarToggleButton() = default;
};

#endif
