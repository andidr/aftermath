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

#ifndef AM_TYPED_LIST_H
#define AM_TYPED_LIST_H

#include "contrib/linux-kernel/list.h"
#include "contrib/linux-kernel/kernel.h"

#define am_typed_list_for_each(c, c_memb, i, i_memb)			\
	for((i) = container_of((c)->c_memb.next, typeof(*(i)), i_memb); \
	    (i) != container_of(&(c)->c_memb, typeof(*(i)), i_memb);	\
	    (i) = container_of((i)->i_memb.next, typeof(*(i)), i_memb))

#define am_typed_list_for_each_prev(c, c_memb, i, i_memb)		\
	for((i) = container_of((c)->c_memb.prev, typeof(*(i)), i_memb); \
	    (i) != container_of(&(c)->c_memb, typeof(*(i)), i_memb);	\
	    (i) = container_of((i)->i_memb.prev, typeof(*(i)), i_memb))

#define am_typed_list_for_each_safe(c, c_memb, i, j, i_memb)		  \
	for((i) = container_of((c)->c_memb.next, typeof(*(i)), i_memb),   \
	      (j) = container_of((i)->i_memb.next, typeof(*(i)), i_memb); \
	    (i) != container_of(&(c)->c_memb, typeof(*(i)), i_memb);	  \
	    (i) = (j),							  \
	      (j) = container_of((i)->i_memb.next, typeof(*(i)), i_memb))

#define am_typed_list_for_each_prev_safe(c, c_memb, i, j, i_memb)	  \
	for((i) = container_of((c)->c_memb.prev, typeof(*(i)), i_memb),   \
	      (j) = container_of((i)->i_memb.prev, typeof(*(i)), i_memb); \
	    (i) != container_of(&(c)->c_memb, typeof(*(i)), i_memb);	  \
	    (i) = (j),							  \
	      (j) = container_of((i)->i_memb.prev, typeof(*(i)), i_memb))

#endif
