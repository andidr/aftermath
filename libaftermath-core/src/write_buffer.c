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

#include "write_buffer.h"
#include <string.h>

/**
 * Initialize data buffer
 * @param size Maximum size of the buffer
 * @return 0 on success, otherwise 1
 */
int am_write_buffer_init(struct am_write_buffer* buf, size_t size)
{
	if(!(buf->data = malloc(size)))
		return 1;

	buf->size = size;
	buf->used = 0;

	return 0;
}

/**
 * Free all resources of a data buffer
 */
void am_write_buffer_destroy(struct am_write_buffer* buf)
{
	free(buf->data);
}

/**
 * Reserve n bytes in the data buffer of an event set. If less than n
 * bytes are available the function returns NULL. Otherwise it returns
 * the pointer to the beginning of the reserved memory region within
 * the buffer.
 */
void* am_write_buffer_reserve_bytes(struct am_write_buffer* buf, size_t n)
{
	void* ret;

	if(buf->size - buf->used < n)
		return NULL;

	ret = buf->data + buf->used;
	buf->used += n;

	return ret;
}

/**
 * Dump all events of a data buffer set to the file fp.
 * @return 0 on success, 1 on failure
 */
int am_write_buffer_dump_fp(struct am_write_buffer* buf, FILE* fp)
{
	if(buf->used == 0)
		return 0;

	if(fwrite(buf->data, buf->used, 1, fp) != 1)
		return 1;

	buf->used = 0;

	return 0;
}

/**
 * Write num_bytes bytes of data to the buffer buf.
 * @return 0 on success, otherwise 1 (e.g., if the buffer is too small)
 */
int am_write_buffer_write_bytes(struct am_write_buffer* buf,
				size_t num_bytes, void* data)
{
	if(buf->size - buf->used < num_bytes)
		return 1;

	memcpy(((char*)buf->data) + buf->used, data, num_bytes);
	buf->used += num_bytes;

	return 0;
}
