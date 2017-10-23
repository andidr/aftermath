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

#ifndef AM_TIMELINE_RENDERER_LAYER_H
#define AM_TIMELINE_RENDERER_LAYER_H

#include <aftermath/core/typed_list.h>
#include <cairo.h>

struct am_timeline_renderer;
struct am_timeline_render_layer;
struct am_timeline_render_layer_type;

/* A time line render layer registry contains one entry per time line render
 * layer type, describing how to allocate, render and destroy a layer.
 */
struct am_timeline_render_layer_type_registry {
	struct list_head types;
};

#define am_timeline_render_layer_type_registry_for_each_type(tr, type) \
	am_typed_list_for_each(tr, types, type, list)

#define am_timeline_render_layer_type_registry_for_each_type_safe(tr, i, n) \
	am_typed_list_for_each_safe(tr, types, i, n, list)

void am_timeline_render_layer_type_registry_init(
	struct am_timeline_render_layer_type_registry* tr);

void am_timeline_render_layer_type_registry_add_type(
	struct am_timeline_render_layer_type_registry* tr,
	struct am_timeline_render_layer_type* t);

void am_timeline_render_layer_type_registry_destroy
(struct am_timeline_render_layer_type_registry* tr);

struct am_timeline_render_layer_type*
am_timeline_render_layer_type_registry_lookup(
	struct am_timeline_render_layer_type_registry* tr,
	const char* name);

struct am_timeline_render_layer*
am_timeline_render_layer_type_registry_instantiate(
	struct am_timeline_render_layer_type_registry* tr,
	const char* name);

typedef void (*am_timeline_render_layer_render_fun_t)(
	struct am_timeline_render_layer* l,
	cairo_t* cr);

typedef void (*am_timeline_render_layer_destroy_fun_t)(
	struct am_timeline_render_layer* l);

typedef struct am_timeline_render_layer*
	(*am_timeline_render_layer_instantiate_fun_t)
	(struct am_timeline_render_layer_type* t);

#define AM_TIMELINE_RENDER_LAYER_RENDER_FUN(x) \
	((am_timeline_render_layer_render_fun_t)(x))

#define AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(x) \
	((am_timeline_render_layer_destroy_fun_t)(x))

#define AM_TIMELINE_RENDER_LAYER_INSTANTIATE_FUN(x) \
	((am_timeline_render_layer_instantiate_fun_t)(x))

/* Description of a render layer type */
struct am_timeline_render_layer_type {
	/* Chanining of all types in the registry */
	struct list_head list;

	char* name;

	am_timeline_render_layer_render_fun_t render;
	am_timeline_render_layer_destroy_fun_t destroy;
	am_timeline_render_layer_instantiate_fun_t instantiate;
};

#define AM_TIMELINE_RENDER_LAYER_TYPE(x) \
	((struct am_timeline_render_layer_type*)x)

int am_timeline_render_layer_type_init(
	struct am_timeline_render_layer_type* l,
	const char* name);

void
am_timeline_render_layer_type_destroy(struct am_timeline_render_layer_type* l);

/* Instance of a render layer */
struct am_timeline_render_layer {
	/* Type of this instance */
	struct am_timeline_render_layer_type* type;

	/* Chaining of all render layers of a time line renderer */
	struct list_head list;

	/* The time line renderer the layer is associated with */
	struct am_timeline_renderer* renderer;
};

#define AM_TIMELINE_RENDER_LAYER(x) \
	((struct am_timeline_render_layer*)x)

void am_timeline_render_layer_init(struct am_timeline_render_layer* l,
				   struct am_timeline_render_layer_type* t);
void am_timeline_render_layer_destroy(struct am_timeline_render_layer* l);

#endif
