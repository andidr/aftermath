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

#ifndef AM_GUI_CONFIGURATION_DIALOG_H
#define AM_GUI_CONFIGURATION_DIALOG_H

#include <QDialog>
#include "ui_GUIConfigurationDialog.h"
#include "../AftermathGUI.h"
#include "../factory/GUIFactory.h"
#include "../widgets/ManagedWidget.h"
#include "../../models/GUITreeModel.h"

/* A dialog that lets the user add, move and remove widgets */
class GUIConfigurationDialog : public QDialog,
			       protected Ui_GUIConfigurationDialog {
	Q_OBJECT

	public:
		GUIConfigurationDialog(AftermathGUI* gui,
				       GUIFactory* factory,
				       WidgetReparenter* reparenter = NULL,
				       WidgetInserter* inserter = NULL,
				       WidgetDeleter* deleter = NULL,
				       QWidget* parent = Q_NULLPTR,
				       Qt::WindowFlags f = Qt::WindowFlags());

		virtual ~GUIConfigurationDialog();

		Q_SIGNAL void createWidget(ManagedContainerWidget* parent,
					   size_t idx,
					   const QString& type);

		void deleteSelection();
		void setReparenter(WidgetReparenter* rp);

	protected:
		void deleteWidgetClicked();
		AftermathGUI* gui;
};

#endif

