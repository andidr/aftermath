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

#include "DFGNodePropertyDialog.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QDialogButtonBox>

DFGNodePropertyDialog::DFGNodePropertyDialog(struct am_dfg_node* n,
					     QWidget* parent,
					     Qt::WindowFlags f) :
	QDialog(parent, f), dfgNode(n)
{
	const struct am_dfg_node_type_functions* nfuns = &n->type->functions;
	struct am_dfg_property* prop;
	QDialogButtonBox* buttonBox;
	QFormLayout* formLayout = NULL;
	QVBoxLayout* mainLayout = NULL;
	QLabel* label;
	QLineEdit* input;
	char* prop_str;
	void* prop_val;
	int cst;

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

	/* Node type must implement get and set property */
	if(!nfuns->get_property || !nfuns->set_property) {
		throw Exception(std::string(n->type->name) + " " +
				"does not implement both get_property and"
				"set_property.");
	}

	am_dfg_node_for_each_property(n, prop) {
		/* The type must implement to_string(), from_string() and
		 * check_string()
		 */
		if(!prop->type->to_string ||
		   !prop->type->from_string ||
		   !prop->type->check_string)
		{
			throw Exception(std::string(prop->type->name) + " " +
					"does not implement all of to_string " +
					"from_string and check_string.");
		}

		label = new QLabel(this);
		label->setText(prop->hrname);
		label->setText(label->text() + ":");

		input = new QLineEdit(this);

		if(nfuns->get_property(n, prop, &prop_val)) {
			throw Exception(std::string("get_property failed on "
						    "property '") +
					n->type->name + "'");
		}

		if(prop->type->to_string(prop->type, prop_val, &prop_str, &cst)){
			throw Exception(std::string("to_string failed on "
						    "property '") +
					prop->name + "'");
		}

		try {
			input->setText(prop_str);
		} catch(...) {
			if(!cst)
				free(prop_str);
		}

		if(!cst)
			free(prop_str);

		formLayout->addRow(label, input);
		this->propEditMap.insert(std::make_pair(prop, input));

		QObject::connect(input, &QLineEdit::textChanged,
				 [=](const QString& txt){
					 this->propertyChanged(prop);
				 });
	}

	buttonBox = new QDialogButtonBox(this);
	buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
	buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
	mainLayout->addWidget(buttonBox);

	this->messageLabel = new QLabel(this);
	mainLayout->addWidget(this->messageLabel);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted,
			 this, &DFGNodePropertyDialog::accepted);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected,
			 this, &DFGNodePropertyDialog::rejected);

	this->setWindowTitle(n->type->hrname);
}

DFGNodePropertyDialog::~DFGNodePropertyDialog()
{
}

/**
 * Called when the value of the line edit associated to a property has
 * changed
 */
void DFGNodePropertyDialog::propertyChanged(struct am_dfg_property* p)
{
	auto it = this->propEditMap.find(p);

	if(it == this->propEditMap.end()) {
		throw Exception("Received propertyChanged signal for property "
				"not managed by this dialog.");
	}

	this->checkProperty(p);
}

/**
 * Checks whether the new value for the property p is valid.
 */
int DFGNodePropertyDialog::checkProperty(struct am_dfg_property* p)
{
	auto it = this->propEditMap.find(p);
	QLineEdit* e = it->second;

	if(!p->type->check_string(p->type, e->text().toUtf8())) {
		e->setStyleSheet("QLineEdit { border: 2px solid rgb(255, 0, 0); }");
		return 0;
	} else {
		e->setStyleSheet("QLineEdit { border: 2px solid rgb(0, 125, 0); }");
		return 1;
	}
}

/**
 * Retrieves the string for the new value of a property p from the associated
 * line edit widget and changes the property accordingly.
 */
void DFGNodePropertyDialog::commitProperty(struct am_dfg_property* p)
{
	auto it = this->propEditMap.find(p);
	QLineEdit* e = it->second;
	void* prop_val;
	auto nfuns = &this->dfgNode->type->functions;

	if(!(prop_val = malloc(p->type->sample_size))) {
		throw Exception(std::string("Could not allocate memory for "
					    "property '") +
				p->name + "'");
	}

	if(p->type->from_string(p->type, e->text().toUtf8(), prop_val)) {
		free(prop_val);

		throw Exception(std::string("Could not create property '") +
				p->name + "' from string");
	}

	if(nfuns->set_property(this->dfgNode, p, prop_val)) {
		if(p->type->destroy_samples)
			p->type->destroy_samples(p->type, 1, prop_val);

		free(prop_val);

		throw Exception(std::string("Could not set property '") +
				p->name + "'");
	}

	if(p->type->destroy_samples)
		p->type->destroy_samples(p->type, 1, prop_val);

	free(prop_val);
}

/**
 * Commits the values for all properties
 */
void DFGNodePropertyDialog::commitAllProperties()
{
	for(auto it: this->propEditMap)
		this->commitProperty(it.first);
}

/**
 * Checks for each property if the string value of the associated line edit
 * widget is a valid, but does not change any property. Returns 1 if all strings
 * are valid, 0 otherwise.
 */
int DFGNodePropertyDialog::checkAllProperties()
{
	int ret = 1;

	for(auto it: this->propEditMap) {
		auto p = it.first;

		if(!this->checkProperty(p))
			ret = 0;
	}

	return ret;
}

void DFGNodePropertyDialog::accepted()
{
	try {
		if(this->checkAllProperties()) {
			this->commitAllProperties();
			this->accept();
		}
	} catch(Exception& e) {
		this->messageLabel->setText(e.what());
		this->messageLabel->setStyleSheet("QLabel { color: red; }");
	}
}

void DFGNodePropertyDialog::rejected()
{
	this->reject();
}
