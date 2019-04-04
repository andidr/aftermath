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

#include "DFGNodeTypeSelectionDialog.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>

DFGNodeTypeSelectionDialog::DFGNodeTypeSelectionDialog(
	struct am_dfg_node_type_registry* ntr,
	QWidget* parent,
	Qt::WindowFlags f) :
	QDialog(parent, f)
{
	QDialogButtonBox* buttonBox;
	QFormLayout* formLayout = NULL;
	QVBoxLayout* mainLayout = NULL;
	QLabel* label;
	QHeaderView* vheader;
	QHeaderView* hheader;

	this->model = new DFGNodeTypeRegistryTableModel(ntr, this);

	try {
		formLayout = new QFormLayout();
		mainLayout = new QVBoxLayout();
		mainLayout->addLayout(formLayout);
	} catch(...) {
		delete formLayout;
		delete mainLayout;
		throw;
	}

	try {
		this->setLayout(mainLayout);
	} catch(...) {
		delete mainLayout;
		throw;
	}

	label = new QLabel(this);
	label->setText("Type:");
	this->edit = new QLineEdit(this);
	this->edit->installEventFilter(this);
	formLayout->addRow(label, this->edit);

	this->table = new QTableView(this);
	this->table->setSizePolicy(QSizePolicy::Expanding,
				   QSizePolicy::Expanding);
	this->table->setModel(this->model);
	this->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	vheader = this->table->verticalHeader();
	vheader->setVisible(false);
	vheader->setSectionResizeMode(QHeaderView::ResizeToContents);
	hheader = this->table->horizontalHeader();
	hheader->setSectionResizeMode(QHeaderView::Stretch);
	this->table->resizeRowsToContents();
	this->table->setTextElideMode(Qt::ElideRight);
	this->table->setWordWrap(false);
	this->table->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->table->setSelectionMode(QAbstractItemView::SingleSelection);
	this->table->setShowGrid(false);
	this->table->setAlternatingRowColors(true);
	this->table->setStyleSheet("alternate-background-color: #EFEFEF;");
	mainLayout->addWidget(this->table);

	buttonBox = new QDialogButtonBox(this);
	buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
	buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
	mainLayout->addWidget(buttonBox);

	this->setWindowTitle("Select node type");
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->adjustSize();

	QObject::connect(buttonBox, &QDialogButtonBox::accepted,
			 this, &DFGNodeTypeSelectionDialog::accepted);
	QObject::connect(this->table, &QTableView::doubleClicked,
			 this, [=](const QModelIndex &idx){ this->accepted(); });
	QObject::connect(buttonBox, &QDialogButtonBox::rejected,
			 this, &DFGNodeTypeSelectionDialog::rejected);
	QObject::connect(this->edit, &QLineEdit::textChanged,
			 this, &DFGNodeTypeSelectionDialog::editChanged);
}

/* Returns the curent selected type. If no type is selected, NULL is
 * returned. */
struct am_dfg_node_type* DFGNodeTypeSelectionDialog::getSelectedType()
{
	QItemSelectionModel* sm = this->table->selectionModel();
	int selectedRow;

	if(!sm->hasSelection() || sm->selectedRows().count() == 0)
		return NULL;

	selectedRow = sm->selectedRows().at(0).row();

	return this->model->getType(selectedRow);
}

DFGNodeTypeSelectionDialog::~DFGNodeTypeSelectionDialog()
{
}

void DFGNodeTypeSelectionDialog::accepted()
{
	if(this->getSelectedType())
		this->accept();
}

void DFGNodeTypeSelectionDialog::rejected()
{
	this->reject();
}

/* Called when either the up arrow of the down arrow is pressed in the line edit
 * for the type name. The selection in the table view is moved up / down
 * accordingly. */
bool DFGNodeTypeSelectionDialog::lineEditUpDownFilter(int key)
{
	QItemSelectionModel* sm = this->table->selectionModel();
	int rowCount = this->model->rowCount();
	int selectedRow;
	int newRow = 0;

	if(rowCount == 0)
		return true;

	if(sm->hasSelection() && sm->selectedRows().count() > 0) {
		selectedRow = sm->selectedRows().at(0).row();

		if(key == Qt::Key_Up) {
			newRow = (selectedRow > 0) ?
				selectedRow - 1 :
				selectedRow;
		} else if(key == Qt::Key_Down) {
			newRow = (selectedRow != rowCount - 1) ?
				selectedRow + 1 :
				selectedRow;
		}
	} else {
		if(key == Qt::Key_Up)
			newRow = rowCount - 1;
		else if(key == Qt::Key_Down)
			newRow = 0;
	}

	this->table->selectRow(newRow);

	return true;
}

/* Event filter for the line edit for the type name. */
bool DFGNodeTypeSelectionDialog::eventFilter(QObject* o, QEvent* e)
{
	QKeyEvent* keyEvent;
	int key;

	if(o != this->edit || e->type() != QEvent::KeyPress)
		return QObject::eventFilter(o, e);

	keyEvent = static_cast<QKeyEvent*>(e);
	key = keyEvent->key();

	if(key == Qt::Key_Up || key == Qt::Key_Down)
		return this->lineEditUpDownFilter(key);
	else
		return QObject::eventFilter(o, e);
}

/* Called when the value of the line edit for the type name changes. Sets the
 * filter according to the value. */
void DFGNodeTypeSelectionDialog::editChanged(const QString& txt)
{
	this->model->setFilterString(txt, Qt::CaseInsensitive);

	if(this->model->rowCount() > 0)
		this->table->selectRow(0);
}
