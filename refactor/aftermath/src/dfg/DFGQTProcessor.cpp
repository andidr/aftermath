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

#include "DFGQTProcessor.h"
#include <iostream>

extern "C" {
	#include <aftermath/core/dfg_schedule.h>
}

DFGQTProcessor::DFGQTProcessor()
	: enabled(true)
{
}

/* Associate a DFG graph with the processor */
void DFGQTProcessor::setDFG(struct am_dfg_graph* g) noexcept
{
	this->dfgGraph = g;
}

/* Returns the currently associated DFG graph */
struct am_dfg_graph* DFGQTProcessor::getDFG() noexcept
{
	return this->dfgGraph;
}

/* Evaluates the associated graph. The node n is the node that triggered the
 * evaluation. */
void DFGQTProcessor::DFGNodeTriggered(struct am_dfg_node* n)
{
	if(!this->dfgGraph)
		return;

	if(!this->enabled)
		return;

	this->disable();

	try {
		am_dfg_schedule(this->dfgGraph);
	} catch(...) {
		this->enable();
		throw;
	}

	this->enable();
}

void DFGQTProcessor::enable()
{
	this->setEnabled(true);
}

void DFGQTProcessor::disable()
{
	this->setEnabled(false);
}

void DFGQTProcessor::setEnabled(bool b)
{
	this->enabled = b;
}

bool DFGQTProcessor::isEnabled()
{
	return this->enabled;
}
