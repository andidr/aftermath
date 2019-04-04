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

#include "HierarchyComboBox.h"
#include "../../dfg/nodes/gui/hierarchy_combobox.h"

HierarchyComboBox::HierarchyComboBox(QWidget* parent,
				     struct am_hierarchyp_array* ha) :
	QComboBox(parent)
{
	this->setEditable(false);
	this->setHierarchies(ha);

	QObject::connect(this, static_cast<void(QComboBox::*)(int)>
			 (&QComboBox::currentIndexChanged),
			 this, [=](int idx){
				 if(this->dfgNode) {
					 this->dfgNode->required_mask.push_new =
						 (1 << AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_HIERARCHY);
					 this->processDFGNode();
				 }
			 });
}

HierarchyComboBox::~HierarchyComboBox()
{
}

struct am_hierarchyp_array* HierarchyComboBox::getHierarchies()
{
	return this->hierarchies;
}

void HierarchyComboBox::setHierarchies(struct am_hierarchyp_array* ha)
{
	this->clear();
	this->hierarchies = ha;

	if(!ha)
		return;

	for(size_t i = 0; i < ha->num_elements; i++)
		this->addItem(ha->elements[i]->name);
}

/* Returns the currently selected hierarchy or NULL if none is selected.
 */
struct am_hierarchy* HierarchyComboBox::getSelectedHierarchy()
{
	int idx = this->currentIndex();

	if(!this->hierarchies || idx == -1 ||
	   (size_t)idx >= this->hierarchies->num_elements)
	{
		return NULL;
	}

	return this->hierarchies->elements[idx];
}
