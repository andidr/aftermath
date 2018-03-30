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

#include "builtin_nodes.h"

#include <aftermath/core/dfg_builtin_node_impl.h>

#define DEFS_NAME() amgui_hierarchy_combobox_defs
#include "gui/hierarchy_combobox.h"

#undef DEFS_NAME
#define DEFS_NAME() amgui_histogram_defs
#include "gui/histogram.h"

#undef DEFS_NAME
#define DEFS_NAME() amgui_label_defs
#include "gui/label.h"

#undef DEFS_NAME
#define DEFS_NAME() amgui_timeline_defs
#include "gui/timeline.h"

/* Final list of all lists of node types from all headers included above */
static struct am_dfg_static_node_type_def** defsets[] = {
	amgui_hierarchy_combobox_defs,
	amgui_histogram_defs,
	amgui_label_defs,
	amgui_timeline_defs,
	NULL
};

/* Register the builtin node types at the node type registry ntr using the type
 * registry tr. Returns 0 on success, otherwise 1. */
int amgui_dfg_builtin_node_types_register(struct am_dfg_node_type_registry* ntr,
					  struct am_dfg_type_registry* tr)
{
	return am_dfg_node_type_registry_add_static(ntr, tr, defsets);
}
