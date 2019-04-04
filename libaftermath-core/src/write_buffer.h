/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * ************************************************************************
 * * THIS FILE IS PART OF THE CODE RELEASED UNDER THE LGPL, VERSION 2.1   *
 * * UNLIKE THE MAJORITY OF THE CODE OF LIBAFTERMATH-CORE, RELEASED UNDER *
 * * THE GPL, VERSION 2.                                                  *
 * ************************************************************************
 *
 * This file can be redistributed it and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_WRITE_BUFFER_H
#define AM_WRITE_BUFFER_H

#include <stdlib.h>
#include <stdio.h>

/* Buffer storing raw event data that will be dumped to the
 * trace file */
struct am_write_buffer {
	/* Pointer to the raw data */
	void* data;

	/* Total size of the data buffer in bytes */
	size_t size;

	/* Number of bytes already used in the data buffer */
	size_t used;
};

int am_write_buffer_init(struct am_write_buffer* buf, size_t size);
void am_write_buffer_destroy(struct am_write_buffer* buf);
void* am_write_buffer_reserve_bytes(struct am_write_buffer* buf, size_t n);
int am_write_buffer_dump_fp(struct am_write_buffer* buf, FILE* fp);
int am_write_buffer_write_bytes(struct am_write_buffer* buf,
				size_t num_bytes,
				void* data);

#endif
