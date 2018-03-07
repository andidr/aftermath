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

#ifndef AM_DFG_TYPE_INTERVAL_H
#define AM_DFG_TYPE_INTERVAL_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/in_memory.h>

int am_dfg_type_interval_to_string(const struct am_dfg_type* t,
				   void* ptr,
				   char** out,
				   int* cst);

int am_dfg_type_interval_from_string(const struct am_dfg_type* t,
				     const char* str,
				     void* out);

int am_dfg_type_interval_check_string(const struct am_dfg_type* t,
				      const char* str);

#define AM_DFG_TYPE_INTERVAL_SAMPLE_SIZE sizeof(struct am_interval)

#endif
