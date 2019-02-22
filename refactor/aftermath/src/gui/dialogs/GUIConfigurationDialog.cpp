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

#include "GUIConfigurationDialog.h"
#include "../../models/WidgetTypeListModel.h"

GUIConfigurationDialog::GUIConfigurationDialog(
	AftermathGUI* gui,
	GUIFactory* factory,
	WidgetReparenter* reparenter,
	WidgetInserter* inserter,
	WidgetDeleter* deleter,
	QWidget* parent,
	Qt::WindowFlags f) :
	QDialog(parent, f), gui(gui)
{
	GUITreeModel* widgetTreeModel = NULL;
	WidgetTypeListModel* widgetTypeListModel = NULL;

	try {
		this->setupUi(this);
		widgetTreeModel = GUITreeModel::build(gui->getRootWidget());
		widgetTypeListModel = new WidgetTypeListModel(factory);
	} catch(...) {
		delete widgetTreeModel;
		delete widgetTypeListModel;
	}

	widgetTreeModel->setReparenter(reparenter);
	widgetTreeModel->setInserter(inserter);
	widgetTreeModel->setDeleter(deleter);

	this->widget_treeview->setModel(widgetTreeModel);
	this->widget_types->setModel(widgetTypeListModel);

	QObject::connect(this->filter_edit, &QLineEdit::textChanged,
			 [=](const QString& txt){
				 widgetTypeListModel->setFilterString(txt);
			 });

	QObject::connect(this->button_remove, &QPushButton::clicked,
			 this, &GUIConfigurationDialog::deleteWidgetClicked);
}

GUIConfigurationDialog::~GUIConfigurationDialog()
{
}

/* Invoked when the widget selected in the widget treeview should be deleted */
void GUIConfigurationDialog::deleteWidgetClicked()
{
	QModelIndex mindex = this->widget_treeview->currentIndex();
	QAbstractItemModel* model = this->widget_treeview->model();
	QModelIndex parentMIndex = mindex.parent();
	int row = mindex.row();

	model->removeRows(row, 1, parentMIndex);
}

/* Removes the current selection from the widget tree view */
void GUIConfigurationDialog::deleteSelection()
{
	QModelIndex index = this->widget_treeview->currentIndex();

	if(index.isValid()) {
		this->widget_treeview->model()->removeRows(
			index.row(), 1, index.parent());
	}
}
