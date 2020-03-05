/**
 * Author: Andi Drebes <andi@drebesium.org>
 * Author: Igor Wodiany <igor.wodiany@manchester.ac.uk>
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

#include "ompt.h"
#include <aftermath/core/in_memory.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/layers/discrete.h>
#include <aftermath/render/timeline/renderer.h>

struct am_color_map ompt_colors = AM_STATIC_COLOR_MAP({
		  AM_RGBA255_EL(229, 115, 115, 255),
			AM_RGBA255_EL(186, 104, 200, 255),
			AM_RGBA255_EL(132, 134, 203, 255),
			AM_RGBA255_EL( 79, 195, 247, 255),
			AM_RGBA255_EL( 77, 182, 172, 255),
			AM_RGBA255_EL(174, 213, 129, 255),
			AM_RGBA255_EL(255, 241, 118, 255),
			AM_RGBA255_EL(255, 183,  77, 255)
		});

static int trace_changed_per_trace_array(struct am_timeline_render_layer* l,
					 struct am_trace* t,
					 const char* array_ident)
{
	struct am_typed_array_generic* arr;
	struct am_timeline_interval_layer* il = (typeof(il))l;
	size_t max_index = 0;

	if(t && (arr = am_trace_find_trace_array(t, array_ident))) {
		max_index = arr->num_elements-1;

		am_timeline_interval_layer_set_extra_data(il, arr);
		am_timeline_interval_layer_set_color_map(
			AM_TIMELINE_INTERVAL_LAYER(l),
			&ompt_colors);
	}

	return am_timeline_interval_layer_set_max_index(
		AM_TIMELINE_INTERVAL_LAYER(l),
		max_index);
}

static int trace_changed_per_ecoll_array(struct am_timeline_render_layer* l,
					 struct am_trace* t)
{
	struct am_timeline_interval_layer* il = (typeof(il))l;

	am_timeline_interval_layer_set_color_map(il, &ompt_colors);

	return am_timeline_interval_layer_set_max_index(
		il, ompt_colors.num_elements-1);
}

static int trace_changed_per_discrete_ecoll_array(struct am_timeline_render_layer* l,
					 struct am_trace* t)
{
	return am_timeline_discrete_layer_set_max_index(
		AM_TIMELINE_DISCRETE_LAYER(l), ompt_colors.num_elements-1);
}

void render_event(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats,
       const short r, const short g, const short b)
{
  cairo_set_source_rgb(cr, r / 255.0, g / 255.0, b / 255.0);
  cairo_rectangle(cr, px, lane_height / 10, 1, 9 * lane_height / 10);
  cairo_rectangle(cr, px, 0, lane_height / 5, lane_height / 10);
  cairo_stroke_preserve(cr);
  cairo_fill(cr);
}

/* Thread region */

static int trace_changed_thread(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_thread(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_thread(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_thread(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_thread* m = arg;

	return 0;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_thread_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::thread",
		"am::ompt::thread",
		sizeof(struct am_ompt_thread),
		offsetof(struct am_ompt_thread, interval),
		calculate_index_thread);

	t->trace_changed = trace_changed_thread;
	t->renderer_changed = renderer_changed_thread;

	return t;
}

/* Parallel region */

static int trace_changed_parallel(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_parallel(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_parallel(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_parallel(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_parallel* m = arg;

	return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_parallel_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::parallel",
		"am::ompt::parallel",
		sizeof(struct am_ompt_parallel),
		offsetof(struct am_ompt_parallel, interval),
		calculate_index_parallel);

	t->trace_changed = trace_changed_parallel;
	t->renderer_changed = renderer_changed_parallel;

	return t;
}

/* Task Create */

void render_task_create(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 240, 98, 146);
}

static int trace_changed_task_create(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_task_create(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_task_create(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_task_create(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_task_create* m = arg;

  // Just some bogus value that has currently no impact on the colour
	return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_task_create_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::task_create",
		"am::ompt::task_create",
		sizeof(struct am_ompt_task_create),
		offsetof(struct am_ompt_task_create, timestamp),
		calculate_index_task_create,
    render_task_create);

	t->trace_changed = trace_changed_task_create;
	t->renderer_changed = renderer_changed_task_create;

	return t;
}

/* Task Schedule */

void render_task_schedule(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 149, 117, 205);
}

static int trace_changed_task_schedule(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_task_schedule(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_task_schedule(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_task_schedule(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_task_schedule* m = arg;

  // Just some bogus value that has currently no impact on the colour
	return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_task_schedule_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::task_schedule",
		"am::ompt::task_schedule",
		sizeof(struct am_ompt_task_schedule),
		offsetof(struct am_ompt_task_schedule, timestamp),
		calculate_index_task_schedule,
    render_task_schedule);

	t->trace_changed = trace_changed_task_schedule;
	t->renderer_changed = renderer_changed_task_schedule;

	return t;
}

/* Implicit task region */

static int trace_changed_implicit_task(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_implicit_task(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_implicit_task(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_implicit_task(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_implicit_task* m = arg;

	return 2;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_implicit_task_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::implicit_task",
		"am::ompt::implicit_task",
		sizeof(struct am_ompt_implicit_task),
		offsetof(struct am_ompt_implicit_task, interval),
		calculate_index_implicit_task);

	t->trace_changed = trace_changed_implicit_task;
	t->renderer_changed = renderer_changed_implicit_task;

	return t;
}

/* Sync region wait */

static int trace_changed_sync_region_wait(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_sync_region_wait(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_sync_region_wait(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_sync_region_wait(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_sync_region_wait* m = arg;

	return 3;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_sync_region_wait_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::sync_region_wait",
		"am::ompt::sync_region_wait",
		sizeof(struct am_ompt_sync_region_wait),
		offsetof(struct am_ompt_sync_region_wait, interval),
		calculate_index_sync_region_wait);

	t->trace_changed = trace_changed_sync_region_wait;
	t->renderer_changed = renderer_changed_sync_region_wait;

	return t;
}

/* Mutex Released */

void render_mutex_released(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 100, 181, 246);
}

static int trace_changed_mutex_released(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_mutex_released(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_mutex_released(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_mutex_released(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_mutex_released* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_mutex_released_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::mutex_released",
		"am::ompt::mutex_released",
		sizeof(struct am_ompt_mutex_released),
		offsetof(struct am_ompt_mutex_released, timestamp),
		calculate_index_mutex_released,
    render_mutex_released);

	t->trace_changed = trace_changed_mutex_released;
	t->renderer_changed = renderer_changed_mutex_released;

	return t;
}

/* Dependences */

void render_dependences(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 77, 208, 225);
}

static int trace_changed_dependences(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_dependences(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_dependences(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_dependences(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_dependences* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_dependences_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::dependences",
		"am::ompt::dependences",
		sizeof(struct am_ompt_dependences),
		offsetof(struct am_ompt_dependences, timestamp),
		calculate_index_dependences,
    render_dependences);

	t->trace_changed = trace_changed_dependences;
	t->renderer_changed = renderer_changed_dependences;

	return t;
}

/* Task Dependence */

void render_task_dependence(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 129, 199, 132);
}

static int trace_changed_task_dependence(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_task_dependence(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_task_dependence(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_task_dependence(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_task_dependence* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_task_dependence_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::task_dependence",
		"am::ompt::task_dependence",
		sizeof(struct am_ompt_task_dependence),
		offsetof(struct am_ompt_task_dependence, timestamp),
		calculate_index_task_dependence,
    render_task_dependence);

	t->trace_changed = trace_changed_task_dependence;
	t->renderer_changed = renderer_changed_task_dependence;

	return t;
}

/* Work region */

static int trace_changed_work(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_work(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_work(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_work(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_work* m = arg;

	return 4;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_work_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::work",
		"am::ompt::work",
		sizeof(struct am_ompt_work),
		offsetof(struct am_ompt_work, interval),
		calculate_index_work);

	t->trace_changed = trace_changed_work;
	t->renderer_changed = renderer_changed_work;

	return t;
}

/* Master region */

static int trace_changed_master(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_master(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_master(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_master(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_master* m = arg;

	return 5;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_master_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::master",
		"am::ompt::master",
		sizeof(struct am_ompt_master),
		offsetof(struct am_ompt_master, interval),
		calculate_index_master);

	t->trace_changed = trace_changed_master;
	t->renderer_changed = renderer_changed_master;

	return t;
}

/* Sync region */

static int trace_changed_sync_region(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_sync_region(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_sync_region(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_sync_region(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_sync_region* m = arg;

	return 6;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_sync_region_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::sync_region",
		"am::ompt::sync_region",
		sizeof(struct am_ompt_sync_region),
		offsetof(struct am_ompt_sync_region, interval),
		calculate_index_sync_region);

	t->trace_changed = trace_changed_sync_region;
	t->renderer_changed = renderer_changed_sync_region;

	return t;
}

/* Lock Init */

void render_lock_init(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 220, 231, 117);
}

static int trace_changed_lock_init(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_lock_init(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_lock_init(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_lock_init(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_lock_init* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_lock_init_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::lock_init",
		"am::ompt::lock_init",
		sizeof(struct am_ompt_lock_init),
		offsetof(struct am_ompt_lock_init, timestamp),
		calculate_index_lock_init,
    render_lock_init);

	t->trace_changed = trace_changed_lock_init;
	t->renderer_changed = renderer_changed_lock_init;

	return t;
}

/* Lock Destroy */

void render_lock_destroy(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 255, 213, 79);
}

static int trace_changed_lock_destroy(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_lock_destroy(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_lock_destroy(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_lock_destroy(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_lock_destroy* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_lock_destroy_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::lock_destroy",
		"am::ompt::lock_destroy",
		sizeof(struct am_ompt_lock_destroy),
		offsetof(struct am_ompt_lock_destroy, timestamp),
		calculate_index_lock_destroy,
    render_lock_destroy);

	t->trace_changed = trace_changed_lock_destroy;
	t->renderer_changed = renderer_changed_lock_destroy;

	return t;
}

/* Mutex Acquire */

void render_mutex_acquire(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 255, 138, 101);
}

static int trace_changed_mutex_acquire(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_mutex_acquire(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_mutex_acquire(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_mutex_acquire(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_mutex_acquire* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_mutex_acquire_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::mutex_acquire",
		"am::ompt::mutex_acquire",
		sizeof(struct am_ompt_mutex_acquire),
		offsetof(struct am_ompt_mutex_acquire, timestamp),
		calculate_index_mutex_acquire,
    render_mutex_acquire);

	t->trace_changed = trace_changed_mutex_acquire;
	t->renderer_changed = renderer_changed_mutex_acquire;

	return t;
}

/* Mutex Acquired */

void render_mutex_acquired(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 224, 224, 224);
}

static int trace_changed_mutex_acquired(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_mutex_acquired(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_mutex_acquired(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_mutex_acquired(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_mutex_acquired* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_mutex_acquired_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::mutex_acquired",
		"am::ompt::mutex_acquired",
		sizeof(struct am_ompt_mutex_acquired),
		offsetof(struct am_ompt_mutex_acquired, timestamp),
		calculate_index_mutex_acquired,
    render_mutex_acquired);

	t->trace_changed = trace_changed_mutex_acquired;
	t->renderer_changed = renderer_changed_mutex_acquired;

	return t;
}

/* Nest Lock */

static int trace_changed_nest_lock(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_ecoll_array(l, t);
}

static int renderer_changed_nest_lock(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_nest_lock(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_nest_lock(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_ompt_nest_lock* m = arg;

	return 7;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_nest_lock_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"ompt::nest_lock",
		"am::ompt::nest_lock",
		sizeof(struct am_ompt_nest_lock),
		offsetof(struct am_ompt_nest_lock, interval),
		calculate_index_nest_lock);

	t->trace_changed = trace_changed_nest_lock;
	t->renderer_changed = renderer_changed_nest_lock;

	return t;
}

/* Flush */

void render_flush(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 255, 255, 255);
}

static int trace_changed_flush(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_flush(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_flush(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_flush(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_flush* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_flush_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::flush",
		"am::ompt::flush",
		sizeof(struct am_ompt_flush),
		offsetof(struct am_ompt_flush, timestamp),
		calculate_index_flush,
    render_flush);

	t->trace_changed = trace_changed_flush;
	t->renderer_changed = renderer_changed_flush;

	return t;
}

/* Cancel */

void render_cancel(
       struct am_timeline_discrete_layer* l,
       const struct am_hierarchy_node* hn,
       const struct am_interval* i,
       double lane_width,
       double lane_height,
       cairo_t* cr,
       unsigned int px,
       const struct am_discrete_stats_by_index* px_stats)
{
  render_event(l, hn, i, lane_width, lane_height, cr, px, px_stats, 144, 164, 174);
}

static int trace_changed_cancel(struct am_timeline_render_layer* l,
					  struct am_trace* t)
{
	return trace_changed_per_discrete_ecoll_array(l, t);
}

static int renderer_changed_cancel(struct am_timeline_render_layer* l,
					     struct am_timeline_renderer* r)
{
	if(r->trace)
		return trace_changed_cancel(l, r->trace);
	else
		return 0;
}

static size_t calculate_index_cancel(
	struct am_timeline_discrete_layer* l,
	void* arg)
{
	struct am_ompt_cancel* m = arg;

	// Just some bogus value that has currently no impact on the colour
  return 1;
}

struct am_timeline_render_layer_type*
am_timeline_ompt_cancel_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_discrete_layer_instantiate_type_index_fun(
		"ompt::cancel",
		"am::ompt::cancel",
		sizeof(struct am_ompt_cancel),
		offsetof(struct am_ompt_cancel, timestamp),
		calculate_index_cancel,
    render_cancel);

	t->trace_changed = trace_changed_cancel;
	t->renderer_changed = renderer_changed_cancel;

	return t;
}
