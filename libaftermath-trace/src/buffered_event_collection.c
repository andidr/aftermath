/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * Libaftermath-trace is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
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

#include "buffered_event_collection.h"

int am_buffered_event_collection_init(
	struct am_buffered_event_collection* bec,
	am_event_collection_id_t id,
	size_t buffer_size)
{
	if(am_write_buffer_init(&bec->data, buffer_size))
		return 1;

	bec->id = id;

	return 0;

}

/**
 * Free all resources of a buffered event collection
 */
void
am_buffered_event_collection_destroy(struct am_buffered_event_collection* bec)
{
	am_write_buffer_destroy(&bec->data);
}

/**
 * Dumps all data from the buffer to an open file
 * @param fp File pointer to the file into which the data is to be dumped; Must
 * be open in write mode
 * @return 0 on success, otherwise 1
 */
int
am_buffered_event_collection_dump_fp(struct am_buffered_event_collection* bec,
				     FILE* fp)
{
	return am_write_buffer_dump_fp(&bec->data, fp);
}
