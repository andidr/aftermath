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

#ifndef AM_TIMELINE_LANE_RENDERER_OMPT_H
#define AM_TIMELINE_LANE_RENDERER_OMPT_H

struct am_timeline_render_layer_type*
am_timeline_ompt_thread_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_parallel_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_task_create_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_task_schedule_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_implicit_task_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_sync_region_wait_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_mutex_released_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_dependences_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_task_dependence_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_work_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_master_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_sync_region_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_lock_init_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_lock_destroy_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_mutex_acquire_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_mutex_acquired_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_nest_lock_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_flush_layer_instantiate_type(void);

struct am_timeline_render_layer_type*
am_timeline_ompt_cancel_layer_instantiate_type(void);

#endif
