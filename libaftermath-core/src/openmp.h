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

#ifndef AM_OPENMP_H
#define AM_OPENMP_H

enum am_openmp_for_flag {
	/* Loop with schedule(static) */
	AM_OPENMP_FOR_TYPE_SCHEDULE_STATIC = (1 << 0),

	/* Loop with schedule(dynamic) */
	AM_OPENMP_FOR_TYPE_SCHEDULE_DYNAMIC = (1 << 1),

	/* Loop with schedule(guided) */
	AM_OPENMP_FOR_TYPE_SCHEDULE_GUIDED = (1 << 2),

	/* Loop with schedule(auto) */
	AM_OPENMP_FOR_TYPE_SCHEDULE_AUTO = (1 << 3),

	/* Loop with schedule(runtime) */
	AM_OPENMP_FOR_TYPE_SCHEDULE_RUNTIME = (1 << 4),

	/* Interpret loop increment as signed integer in two's complement */
	AM_OPENMP_FOR_TYPE_SIGNED_INCREMENT = (1 << 5),

	/* Interpret loop bounds as signed integers in two's complement */
	AM_OPENMP_FOR_TYPE_SIGNED_ITERATION_SPACE = (1 << 6),

	/* Loop has clause for explicit chunk size */
	AM_OPENMP_FOR_TYPE_CHUNKED = (1 << 7)
};

#endif
