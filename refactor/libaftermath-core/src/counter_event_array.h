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

#ifndef AM_COUNTER_EVENT_ARRAY_H
#define AM_COUNTER_EVENT_ARRAY_H

#include "typed_array.h"
#include "in_memory.h"

#define AM_COUNTER_EVENT_ARRAY_EXTRA_FIELDS \
	am_counter_t counter_id;

AM_DECL_TYPED_ARRAY_EXTRA_FIELDS(am_counter_event_array,
				 struct am_counter_event,
				 AM_COUNTER_EVENT_ARRAY_EXTRA_FIELDS)

#endif
