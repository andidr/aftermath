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

#include <aftermath/core/dfg_builtin_type_impl.h>
#include <aftermath/render/cairo_extras.h>
#include <aftermath/render/dfg/types/builtin_types.h>
#include <aftermath/render/timeline/layer.h>

AM_DFG_DECL_BUILTIN_TYPE(
	am_render_dfg_type_timeline_layer,
	"const am::render::timeline::layer*",
	sizeof(struct am_timeline_render_layer*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_render_dfg_type_rgba,
	"am::render::rgba",
	sizeof(struct am_rgba),
	NULL, NULL, NULL, NULL)

static struct am_dfg_static_type_def* builtin_defs[] = {
	&am_render_dfg_type_timeline_layer,
	&am_render_dfg_type_rgba,
	NULL
};

static struct am_dfg_static_type_def** defsets[] = {
	builtin_defs,
	NULL
};

/* Registers all builtin types for libaftermath-render.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_render_dfg_builtin_types_register(struct am_dfg_type_registry* tr)
{
	return am_dfg_type_registry_add_static(tr, defsets);
}
