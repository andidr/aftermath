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

#include "DefaultGUIFactory.h"
#include "../../AftermathSession.h"
#include "BoxWidgetCreator.h"
#include "DFGWidgetCreator.h"
#include "SplitterWidgetCreator.h"
#include "TimelineWidgetCreator.h"

DefaultGUIFactory::DefaultGUIFactory(class AftermathSession* session) :
	GUIFactory(), session(session)
{
	this->addCreator(new HBoxWidgetCreator());
	this->addCreator(new VBoxWidgetCreator());
	this->addCreator(new DFGWidgetCreator());
	this->addCreator(new HSplitterWidgetCreator());
	this->addCreator(new VSplitterWidgetCreator());
	this->addCreator(new TimelineWidgetCreator(session->getRenderLayerTypeRegistry()));
}

DefaultGUIFactory::~DefaultGUIFactory()
{
}
