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

#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "AftermathController.h"
#include "AftermathSession.h"
#include "gui/factory/DefaultGUIFactory.h"
#include "gui/widgets/DFGWidget.h"
#include "gui/widgets/CairoWidgetWithDFGNode.h"

extern "C" {
	#include <aftermath/core/dfg/nodes/trace.h>
}

int main(int argc, char *argv[])
{
	AftermathSession session;
	QApplication a(argc, argv);
	MainWindow mainWindow;
	DefaultGUIFactory factory(&session);

	AftermathGUI& gui = session.getGUI();

	session.loadTrace(argv[1]);
	factory.buildGUI(&gui, argv[2]);
	session.loadDFG(argv[3]);

	AftermathController controller(&session, &mainWindow);

	mainWindow.show();

	session.scheduleDFG();

	return a.exec();
}
