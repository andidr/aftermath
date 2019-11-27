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

#include "openmp.h"
#include <aftermath/core/in_memory.h>
#include <aftermath/core/openmp_for_loop_type_array.h>
#include <aftermath/core/openmp_for_loop_instance_array.h>
#include <aftermath/core/openmp_iteration_set_array.h>
#include <aftermath/core/openmp_iteration_period_array.h>
#include <aftermath/core/openmp_task_type_array.h>
#include <aftermath/core/openmp_task_instance_array.h>
#include <aftermath/core/openmp_task_period_array.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/renderer.h>

struct am_color_map openmp_colors = AM_STATIC_COLOR_MAP({
		AM_RGBA255_EL(117, 195, 255, 255),
			AM_RGBA255_EL(  0,   0, 255, 255),
			AM_RGBA255_EL(255, 255, 255, 255),
			AM_RGBA255_EL(255,   0,   0, 255),
			AM_RGBA255_EL(255,   0, 174, 255),
			AM_RGBA255_EL(179,   0,   0, 255),
			AM_RGBA255_EL(  0, 255,   0, 255),
			AM_RGBA255_EL(255, 255,   0, 255),
			AM_RGBA255_EL(235,   0,   0, 255)
		});

static int trace_changed_per_trace_array(struct am_timeline_render_layer* l,
					 struct am_trace* t,
					 const char* array_ident)
{
	struct am_typed_array_generic* arr;
	struct am_timeline_interval_layer* il = (typeof(il))l;

	if(!t || !(arr = am_trace_find_trace_array(t, array_ident)))
		return 0;

	am_timeline_interval_layer_set_extra_data(il, arr);
	am_timeline_interval_layer_set_color_map(AM_TIMELINE_INTERVAL_LAYER(l),
						 &openmp_colors);

	if(arr->num_elements > 0) {
		return am_timeline_interval_layer_set_max_index(
			AM_TIMELINE_INTERVAL_LAYER(l),
			arr->num_elements-1);
	}

	return 0;
}

static int trace_changed_per_ecoll_array(struct am_timeline_render_layer* l,
					 struct am_trace* t)
{
	struct am_timeline_interval_layer* il = (typeof(il))l;

	am_timeline_interval_layer_set_color_map(il, &openmp_colors);

	return am_timeline_interval_layer_set_max_index(
		il, openmp_colors.num_elements-1);
}


/* For loop type */

static int trace_changed_loop_type(struct am_timeline_render_layer* l,
				   struct am_trace* t)
{
	return trace_changed_per_trace_array(l, t, "am::openmp::for_loop_type");
}

static int renderer_changed_loop_type(struct am_timeline_render_layer* l,
				      struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_loop_type(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_loop_type(struct am_timeline_interval_layer* l, void* arg)
{
	struct am_openmp_iteration_period* ip = arg;
	struct am_openmp_for_loop_type_array* larr;
	struct am_openmp_for_loop_type* t;

	t = ip->iteration_set->loop_instance->loop_type;
	larr = am_timeline_interval_layer_get_extra_data(l);

	return am_openmp_for_loop_type_array_index(larr, t);
}

struct am_timeline_render_layer_type*
am_timeline_openmp_for_loop_type_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::for_loop_type",
		"am::openmp::iteration_period",
		sizeof(struct am_openmp_iteration_period),
		offsetof(struct am_openmp_iteration_period, interval),
		calculate_index_loop_type);

	t->trace_changed = trace_changed_loop_type;
	t->renderer_changed = renderer_changed_loop_type;

	return t;
}

/* For loop instance */

static int trace_changed_loop_instance(struct am_timeline_render_layer* l,
				       struct am_trace* t)
{
	return trace_changed_per_trace_array(
		l, t, "am::openmp::for_loop_instance");
}

static int renderer_changed_loop_instance(struct am_timeline_render_layer* l,
					  struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_loop_instance(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_loop_instance(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_openmp_iteration_period* ip = arg;
	struct am_openmp_for_loop_instance_array* larr;
	struct am_openmp_for_loop_instance* t;

	t = ip->iteration_set->loop_instance;
	larr = am_timeline_interval_layer_get_extra_data(l);

	return am_openmp_for_loop_instance_array_index(larr, t);
}

struct am_timeline_render_layer_type*
am_timeline_openmp_for_loop_instance_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::for_loop_instance",
		"am::openmp::iteration_period",
		sizeof(struct am_openmp_iteration_period),
		offsetof(struct am_openmp_iteration_period, interval),
		calculate_index_loop_instance);

	t->trace_changed = trace_changed_loop_instance;
	t->renderer_changed = renderer_changed_loop_instance;

	return t;
}

/* Iteration sets */

static int trace_changed_iteration_set(struct am_timeline_render_layer* l,
				       struct am_trace* t)
{
	return trace_changed_per_trace_array(l, t, "am::openmp::iteration_set");
}

static int renderer_changed_iteration_set(struct am_timeline_render_layer* l,
					  struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_iteration_set(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_iteration_set(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_openmp_iteration_period* ip = arg;
	struct am_openmp_iteration_set_array* larr;
	struct am_openmp_iteration_set* t;

	t = ip->iteration_set;
	larr = am_timeline_interval_layer_get_extra_data(l);

	return am_openmp_iteration_set_array_index(larr, t);
}

struct am_timeline_render_layer_type*
am_timeline_openmp_iteration_set_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::iteration_set",
		"am::openmp::iteration_period",
		sizeof(struct am_openmp_iteration_period),
		offsetof(struct am_openmp_iteration_period, interval),
		calculate_index_iteration_set);

	t->trace_changed = trace_changed_iteration_set;
	t->renderer_changed = renderer_changed_iteration_set;

	return t;
}

/* Iteration periods */

static int trace_changed_iteration_period(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_iteration_period(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_iteration_period(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_iteration_period(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_openmp_iteration_period* ip = arg;

	/* Some arbitrary expression with reproducible indexes across for the
	 * same trace */
	return ((size_t)ip->interval.start * ip->interval.end) %
		openmp_colors.num_elements;
}

struct am_timeline_render_layer_type*
am_timeline_openmp_iteration_period_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::iteration_period",
		"am::openmp::iteration_period",
		sizeof(struct am_openmp_iteration_period),
		offsetof(struct am_openmp_iteration_period, interval),
		calculate_index_iteration_period);

	t->trace_changed = trace_changed_iteration_period;
	t->renderer_changed = renderer_changed_iteration_period;

	return t;
}

/* Task type */

static int trace_changed_task_type(struct am_timeline_render_layer* l,
				   struct am_trace* t)
{
	return trace_changed_per_trace_array(l, t, "am::openmp::task_type");
}

static int renderer_changed_task_type(struct am_timeline_render_layer* l,
				      struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_task_type(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_task_type(struct am_timeline_interval_layer* l, void* arg)
{
	struct am_openmp_task_period* ip = arg;
	struct am_openmp_task_type_array* larr;
	struct am_openmp_task_type* t;

	t = ip->task_instance->task_type;
	larr = am_timeline_interval_layer_get_extra_data(l);

	return am_openmp_task_type_array_index(larr, t);
}

struct am_timeline_render_layer_type*
am_timeline_openmp_task_type_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::task_type",
		"am::openmp::task_period",
		sizeof(struct am_openmp_iteration_period),
		offsetof(struct am_openmp_iteration_period, interval),
		calculate_index_task_type);

	t->trace_changed = trace_changed_task_type;
	t->renderer_changed = renderer_changed_task_type;

	return t;
}

/* Task instances */

static int trace_changed_task_instance(struct am_timeline_render_layer* l,
				       struct am_trace* t)
{
	return trace_changed_per_trace_array(l, t, "am::openmp::task_instance");
}

static int renderer_changed_task_instance(struct am_timeline_render_layer* l,
					  struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_task_instance(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_task_instance(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_openmp_task_period* ip = arg;
	struct am_openmp_task_instance_array* larr;
	struct am_openmp_task_instance* t;

	t = ip->task_instance;
	larr = am_timeline_interval_layer_get_extra_data(l);

	return am_openmp_task_instance_array_index(larr, t);
}

struct am_timeline_render_layer_type*
am_timeline_openmp_task_instance_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::task_instance",
		"am::openmp::task_period",
		sizeof(struct am_openmp_task_period),
		offsetof(struct am_openmp_task_period, interval),
		calculate_index_task_instance);

	t->trace_changed = trace_changed_task_instance;
	t->renderer_changed = renderer_changed_task_instance;

	return t;
}

/* Task periods */

static int trace_changed_task_period(struct am_timeline_render_layer* l,
				     struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_task_period(struct am_timeline_render_layer* l,
					struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_task_period(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_task_period(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_openmp_task_period* tp = arg;

	/* Some arbitrary expression with reproducible indexes across for the
	 * same trace */
	return ((size_t)tp->interval.start * tp->interval.end) %
		openmp_colors.num_elements;
}

struct am_timeline_render_layer_type*
am_timeline_openmp_task_period_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"openmp::task_period",
		"am::openmp::task_period",
		sizeof(struct am_openmp_task_period),
		offsetof(struct am_openmp_task_period, interval),
		calculate_index_task_period);

	t->trace_changed = trace_changed_task_period;
	t->renderer_changed = renderer_changed_task_period;

	return t;
}
