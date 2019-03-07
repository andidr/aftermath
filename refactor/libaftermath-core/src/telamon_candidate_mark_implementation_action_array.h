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

#ifndef AM_TELAMON_CANDIDATE_MARK_IMPLEMENTATION_ACTION_ARRAY_H
#define AM_TELAMON_CANDIDATE_MARK_IMPLEMENTATION_ACTION_ARRAY_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/in_memory.h>

AM_DECL_TYPED_ARRAY(am_telamon_candidate_mark_implementation_action_array,
		    struct am_telamon_candidate_mark_implementation_action)

AM_DECL_TYPED_ARRAY(am_telamon_candidate_mark_implementation_actionp_array,
		    struct am_telamon_candidate_mark_implementation_action*)

#endif
