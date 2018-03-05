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

#include "CairoWidgetWithDFGNode.h"

CairoWidgetWithDFGNode::CairoWidgetWithDFGNode(QWidget* parent) :
	super(parent), dfgNode(NULL)
{
}

CairoWidgetWithDFGNode::~CairoWidgetWithDFGNode()
{
}

/**
 * Associates a DFG node with the widgets
 */
void CairoWidgetWithDFGNode::setDFGNode(struct am_dfg_node* n) noexcept
{
	this->dfgNode = n;
}

/**
 * Returns the currently associated DFG node of the widget
 */
struct am_dfg_node* CairoWidgetWithDFGNode::getDFGNode() noexcept
{
	return this->dfgNode;
}

/**
 * Triggers processing of the node. If the node is actually processed depends on
 * whether the processDFGNodeSignal of the widget is connected with an
 * appropriate processor.
 */
void CairoWidgetWithDFGNode::processDFGNode()
{
	if(this->dfgNode)
		emit processDFGNodeSignal(this->dfgNode);
}
