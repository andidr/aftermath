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

#ifndef AM_OPENSTREAM_TASK_PERIOD_ARRAY_H
#define AM_OPENSTREAM_TASK_PERIOD_ARRAY_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/in_memory.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/interval_array.h>

AM_DECL_TYPED_ARRAY(am_openstream_task_period_array, struct am_openstream_task_period)

AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(am_openstream_task_period_array,
						       struct am_openstream_task_period,
						       interval)

AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_LAST_OVERLAPPING(am_openstream_task_period_array,
						      struct am_openstream_task_period,
						      interval)

#endif
