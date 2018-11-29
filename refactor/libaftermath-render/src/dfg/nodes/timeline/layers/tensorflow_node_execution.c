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

#include <aftermath/render/dfg/nodes/timeline/layers/tensorflow_node_execution.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/renderer.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	tensorflow_node_execution,
	"tensorflow::node_execution",
	struct am_timeline_interval_layer)
