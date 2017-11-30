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

#ifndef AM_PTR_H
#define AM_PTR_H

#include <stddef.h>

/* Safe comparisons of pointers */
#define AM_PTR_LESS(a, b) (((uintptr_t)(a)) < ((uintptr_t)(b)))
#define AM_PTR_LEQ(a, b) (((uintptr_t)(a)) <= ((uintptr_t)(b)))
#define AM_PTR_GREATER(a, b) (((uintptr_t)(a)) > ((uintptr_t)(b)))
#define AM_PTR_GEQ(a, b) (((uintptr_t)(a)) >= ((uintptr_t)(b)))

#endif
