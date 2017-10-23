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

#ifndef AM_TIMELINE_HIERARCHY_LAYER_H
#define AM_TIMELINE_HIERARCHY_LAYER_H

#include "../layer.h"

/* The hierarchy render layer renders the hierarchy associated to a time line as
 * a tree-like structure in the left part of the time line, providing each lane
 * with the name of the associated hierarchy node. Example for a hierarchy of a
 * systems with two NUMA nodes, each with 2 CPUs with 2 SMT cores:
 *
 *      NUMA Node 0     Core 0   SMT 0
 *  O---O---------------O--------O
 *  |   |               |
 *  |   |               |        SMT 1
 *  |   |               +--------O
 *  |   |
 *  |   |               Core 1   SMT 0
 *  |   +---------------O--------O
 *  |                   |
 *  |                   |        SMT 1
 *  |                   +--------O
 *  |
 *  |   NUMA Node 0     Core 0   SMT 0
 *  +---O---------------O--------O
 *      |               |
 *      |               |        SMT 1
 *      |               +--------O
 *      |
 *      |               Core 1   SMT 0
 *      +---------------O--------O
 *                      |
 *                      |        SMT 1
 *                      +--------O
 */

struct am_timeline_render_layer_type*
am_timeline_hierarchy_layer_instantiate_type(void);

#endif
