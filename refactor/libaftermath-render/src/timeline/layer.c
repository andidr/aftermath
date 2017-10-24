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

#include "layer.h"
#include <stdlib.h>
#include <string.h>

void am_timeline_render_layer_type_registry_init(
	struct am_timeline_render_layer_type_registry* tr)
{
	INIT_LIST_HEAD(&tr->types);
}

/* Adds a new type description to the registry. Ownership for the type
 * description is transferred to the registry and the type description is
 * desytroyed upon destruction of the registry. */
void am_timeline_render_layer_type_registry_add_type(
	struct am_timeline_render_layer_type_registry* tr,
	struct am_timeline_render_layer_type* t)
{
	list_add_tail(&t->list, &tr->types);
}

void am_timeline_render_layer_type_registry_destroy(
	struct am_timeline_render_layer_type_registry* tr)
{
	struct am_timeline_render_layer_type* type;
	struct am_timeline_render_layer_type* next;

	am_timeline_render_layer_type_registry_for_each_type_safe(tr, type, next) {
		am_timeline_render_layer_type_destroy(type);
		free(type);
	}
}

/* Finds the description of a type by name. Returns the type description or NULL
 * if no such type is known to the type registry. */
struct am_timeline_render_layer_type*
am_timeline_render_layer_type_registry_lookup(
	struct am_timeline_render_layer_type_registry* tr,
	const char* name)
{
	struct am_timeline_render_layer_type* type;

	am_timeline_render_layer_type_registry_for_each_type(tr, type)
		if(strcmp(type->name, name) == 0)
			return type;

	return NULL;
}

/* Create a new instance of a render layer of the type specified by "name". If
 * no such type exists or if the initialization fails, the function returns
 * NULL. Otherwise, a pointer to the new instance is returned. */
struct am_timeline_render_layer*
am_timeline_render_layer_type_registry_instantiate(
	struct am_timeline_render_layer_type_registry* tr,
	const char* name)
{
	struct am_timeline_render_layer_type* t;
	struct am_timeline_render_layer* l;

	if(!(t = am_timeline_render_layer_type_registry_lookup(tr, name)))
		return NULL;

	if(!(l = t->instantiate(t)))
		return NULL;

	l->type = t;

	return l;
}

/* Initialize a render layer type. The name is duplicated and can safely be
 * freed after the call. */
int am_timeline_render_layer_type_init(struct am_timeline_render_layer_type* t,
				       const char* name)
{
	memset(t, 0, sizeof(*t));

	INIT_LIST_HEAD(&t->list);

	if(!(t->name = strdup(name)))
		return 1;

	return 0;
}

void am_timeline_render_layer_type_destroy(
	struct am_timeline_render_layer_type* t)
{
	free(t->name);
}

/* Initialization function for a render layer. To be called from the
 * type-specific initialization function. */
void am_timeline_render_layer_init(struct am_timeline_render_layer* l,
				   struct am_timeline_render_layer_type* t)
{
	INIT_LIST_HEAD(&l->list);
	l->renderer = NULL;
	l->type = t;
}

/* Generic interface to destroy a render layer; invokes the layer's
 * type-specific destructor. */
void am_timeline_render_layer_destroy(struct am_timeline_render_layer* l)
{
	if(l->type->destroy)
		l->type->destroy(l);
}

void am_timeline_entity_init(struct am_timeline_entity* e,
			     struct am_timeline_render_layer* l,
			     int type)
{
	e->layer = l;
	e->type = type;
	INIT_LIST_HEAD(&e->list);
}

void am_timeline_entity_destroy(struct am_timeline_entity* e)
{
}

/* Add entity e at the end of the list lst. */
void am_timeline_entity_append(struct am_timeline_entity* e,
			       struct list_head* lst)
{
	list_add_tail(&e->list, lst);
}
