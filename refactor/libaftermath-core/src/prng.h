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

#ifndef AM_PRNG_H
#define AM_PRNG_H

#include <stdint.h>

struct am_prng_u64 {
	uint64_t value;
};

/* Sets the seed of the PRNG to s. */
static inline void am_prng_u64_seed(struct am_prng_u64* p, uint64_t s)
{
	p->value = s;
}

/* Generates the next pseudo random number using a simple, linear congruental
 * PRNG. Parameters taken from TAOCP. */
static inline void
am_prng_u64_next(struct am_prng_u64* p)
{
	p->value = (p->value * UINT64_C(6364136223846793005)) +
		UINT64_C(1442695040888963407);
}

/* Generates a pseudo random number in [min; max]. */
static inline uint64_t
am_prng_u64_rand(struct am_prng_u64* p, uint64_t min, uint64_t max)
{
	uint64_t w;

	am_prng_u64_next(p);

	if(max - min == UINT64_MAX)
		w = max - min;
	else
		w = max - min + 1;

	return min + (p->value % w);
}

#endif
