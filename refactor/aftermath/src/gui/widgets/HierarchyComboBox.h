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

#ifndef AM_HIERARCHY_COMBOBOX_H
#define AM_HIERARCHY_COMBOBOX_H

#include <QComboBox>
#include "WidgetWithDFGNode.h"

extern "C" {
	#include <aftermath/core/hierarchy.h>
	#include <aftermath/core/hierarchy_array.h>
}

/* A non-editable combo box widget that lets the user select a hierarchy from
 * a hierarchy array.
 */
class HierarchyComboBox : public QComboBox {
	AM_WIDGETWITHDFGNODE_DECLS
	Q_OBJECT

	public:
		HierarchyComboBox(QWidget* parent = NULL,
				  struct am_hierarchyp_array* ha = NULL);
		virtual ~HierarchyComboBox();

		struct am_hierarchyp_array* getHierarchies();
		void setHierarchies(struct am_hierarchyp_array* ha);

		struct am_hierarchy* getSelectedHierarchy();

	protected:
		struct am_hierarchyp_array* hierarchies;
};

#endif
