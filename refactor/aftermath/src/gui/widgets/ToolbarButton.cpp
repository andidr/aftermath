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

#include "ToolbarButton.h"
#include "../../dfg/nodes/gui/toolbar_togglebutton.h"

ToolbarButton::ToolbarButton(QWidget* parent)
	: QToolButton(parent)
{
	QObject::connect(this, static_cast<void(QToolButton::*)(bool)>
			 (&QToolButton::clicked),
			 this, [=](bool){ this->processDFGNode(); });
}

ToolbarToggleButton::ToolbarToggleButton(QWidget* parent)
	: ToolbarButton(parent)
{
	QObject::connect(this, static_cast<void(QToolButton::*)(bool)>
			 (&QToolButton::toggled),
			 this,
			 [=](bool){
				 if(this->dfgNode) {
					 this->dfgNode->required_mask.push_new =
						 (1 << AM_DFG_AMGUI_TOOLBAR_TOGGLEBUTTON_NODE_TOGGLED);
					 this->processDFGNode();
				 }
			 });
}
