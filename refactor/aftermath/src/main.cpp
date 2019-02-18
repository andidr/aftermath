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
#include "Exception.h"
#include "gui/factory/DefaultGUIFactory.h"
#include "gui/widgets/DFGWidget.h"
#include "gui/widgets/CairoWidgetWithDFGNode.h"
#include <QFileInfo>

extern "C" {
	#include <aftermath/core/dfg/nodes/trace.h>
	#include <getopt.h>
}

#include <iostream>
#include <string>

/* Options for the main executable */
struct am_options {
	public:
		std::string profile_name;
		std::string trace_filename;
		std::string dfg_filename;
		std::string ui_filename;
		bool print_usage;
		bool dfg_safe_mode;
};

static void print_usage(void)
{
	std::cout << "Aftermath, a graphical tool for trace-based performance "
		"analysis of parallel programs.\n"
		"\n"
		"  Usage: aftermath [-p profile_path] [-d dfg_file] [-u ui_file] trace_file\n"
		"\n"
		"  -h             Display this help message.\n"
		"  -p profile     Load DFG and user interface from the profile with the given\n"
		"                 name.\n"
		"  -d dfg_file    Load DFG definition from dfg_file.\n"
		"  -u ui_file     Load user interface from ui_file.\n"
		"  -s             Ignore errors during initial scheduling of DFG.\n";
}

/* Parses the options from the argument list argv and sets the options in o
 * accordingly. Throws an exception if parsing fails.
 */
static void parse_options(struct am_options* o, int argc, char** argv)
{
	static const char* options_str = "hd:p:su:";
	int opt;

	/* Default values */
	o->trace_filename = "";
	o->dfg_filename = "";
	o->ui_filename = "";
	o->print_usage = false;
	o->profile_name = "";
	o->dfg_safe_mode = false;

	opterr = 0;

	while((opt = getopt(argc, argv, options_str)) != -1) {
		switch(opt) {
			case 'd':
				o->dfg_filename = optarg;
				break;
			case 'p':
				o->profile_name = optarg;
				break;
			case 's':
				o->dfg_safe_mode = true;
				break;
			case 'u':
				o->ui_filename = optarg;
				break;
			case 'h':
				o->print_usage = 1;
				break;
			default:
				throw AftermathException(
					std::string("Unknown option \"") +
					argv[optind-1] +
					"\"");
		}
	}

	if(argc > 0 && optind < argc) {
		o->trace_filename = argv[optind];
		optind++;
	}

	if(optind != argc)
		throw AftermathException("Excess arguments provided.");
}

/* Checks if the provided options are consistent. Throws an exception
 * in case of inconsistency. */
static void check_options(struct am_options* o)
{
	std::string profile_dir;

	if(o->print_usage)
		return;

	if(o->profile_name == "" &&
	   o->ui_filename == "" &&
	   o->dfg_filename == "")
	{
		o->profile_name = "default";
	}

	if(o->profile_name != "") {
		profile_dir = std::string(AM_PROFILE_BASEDIR) +
			"/" + o->profile_name;
		o->dfg_filename = profile_dir + "/graph.dfg";
		o->ui_filename = profile_dir + "/interface.amgui";
	}

	if(o->ui_filename == "")
		throw AftermathException("No UI filename given.");

	if(o->dfg_filename == "")
		throw AftermathException("No DFG filename given.");

	if(o->trace_filename == "")
		throw AftermathException("No trace filename given.");
}

int aftermath_main(const struct am_options* o,
		   int argc,
		   char *argv[])
{
	QApplication a(argc, argv);

	try {
		MainWindow mainWindow;
		AftermathSession session;
		DefaultGUIFactory factory(&session);

		AftermathGUI& gui = session.getGUI();

		session.loadTrace(o->trace_filename.c_str());
		factory.buildGUI(&gui, o->ui_filename.c_str());
		session.loadDFG(o->dfg_filename.c_str());

		AftermathController controller(&session, &mainWindow);

		QString title = "Aftermath";

		if(o->profile_name != "")
			title += QString(" [") + o->profile_name.c_str() + "]";

		QFileInfo fi(o->trace_filename.c_str());
		title += QString(": ") + fi.fileName();

		mainWindow.setWindowTitle(title);
		mainWindow.show();

		try {
			session.scheduleDFG();
		} catch(AftermathSession::DFGSchedulingException& e) {
			if(!o->dfg_safe_mode)
				throw;
		} catch(...) {
			throw;
		}

		return a.exec();
	} catch(std::exception& e) {
		AftermathController::showError(e.what());
		throw;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	struct am_options options;

	try {
		parse_options(&options, argc, argv);
		check_options(&options);

		if(options.print_usage) {
			print_usage();
			return 0;
		}

		return aftermath_main(&options, argc, argv);
	} catch(std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl
			  << "Exiting." << std::endl;
	} catch(...) {
		std::cerr << "An unhandled exception occurred. Exiting."
			  << std::endl;
	}

	return 1;
}
