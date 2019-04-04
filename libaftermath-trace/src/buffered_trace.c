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

#include <aftermath/trace/buffered_trace.h>
#include <aftermath/trace/on_disk_write_to_buffer.h>
#include <aftermath/trace/safe_alloc.h>

/**
 * Initialize a buffered trace
 * @param buffer_size The size in bytes for the buffer for trace global data
 * @return 0 on success, 1 otherwise
 */
int am_buffered_trace_init(struct am_buffered_trace* bt, size_t buffer_size)
{
	struct am_dsk_header hdr = {
		.magic = AM_TRACE_MAGIC,
		.version = AM_TRACE_VERSION
	};

	if(am_write_buffer_init(&bt->data, buffer_size))
		goto out_err;

	bt->num_collections = 0;
	bt->collections = NULL;
	bt->highest_collection_id = 0;

	bt->num_hierarchies = 0;
	bt->hierarchies = NULL;
	bt->highest_hierarchy_id = 0;

	if(am_dsk_header_write_to_buffer(&bt->data, &hdr))
		goto out_err_destroy;

	return 0;

out_err_destroy:
	am_write_buffer_destroy(&bt->data);
out_err:
	return 1;
}

/**
 * Free all resources associated to a trace buffer
 */
void am_buffered_trace_destroy(struct am_buffered_trace* bt)
{
	for(size_t i = 0; i < bt->num_collections; i++) {
		am_buffered_event_collection_destroy(bt->collections[i]);
		free(bt->collections[i]);
	}

	free(bt->collections);

	for(size_t i = 0; i < bt->num_hierarchies; i++) {
		am_simple_hierarchy_destroy(bt->hierarchies[i]);
		free(bt->hierarchies[i]);
	}

	free(bt->hierarchies);


	am_write_buffer_destroy(&bt->data);
}

/**
 * Write the contents of the entire trace to a file already opened
 * @return 0 on sucess, 1 on failure
 */
int am_buffered_trace_dump_fp(struct am_buffered_trace* bt, FILE* fp)
{
	if(am_write_buffer_dump_fp(&bt->data, fp))
		return 1;

	for(size_t i = 0; i < bt->num_collections; i++)
		if(am_buffered_event_collection_dump_fp(bt->collections[i], fp))
			return 1;

	return 0;
}

/**
 * Write the contents of the entire trace to a file
 * @return 0 on sucess, 1 on failure
 */
int am_buffered_trace_dump(struct am_buffered_trace* bt, const char* filename)
{
	FILE* fp;
	int ret = 1;

	if(!(fp = fopen(filename, "wb+")))
		goto out;

	if(am_buffered_trace_dump_fp(bt, fp))
		goto out_fp;

	ret = 0;
out_fp:
	fclose(fp);
out:
	return ret;
}

/**
 * Adds the buffered event collection bec to the list of event collections of
 * bt. Does not check if a collection with the same ID already exists.
 *
 * @return 0 on success, otherwise 1.
 */
int am_buffered_trace_add_collection(struct am_buffered_trace* bt,
				     struct am_buffered_event_collection* bec)
{
	void* tmp;

	if(!(tmp = am_grow_array_safe(bt->collections,
				      bt->num_collections,
				      1,
				      sizeof(bt->collections[0]))))
	{
		return 1;
	}

	bt->collections = tmp;
	bt->collections[bt->num_collections] = bec;
	bt->num_collections++;

	if(bec->id > bt->highest_collection_id)
		bt->highest_collection_id = bec->id;

	return 0;
}

/**
 * Creates and initializes a new buffered event collection and adds it to the
 * list of event collections.
 *
 * @param buffer_size Size of the buffer for the buffered event collection
 * @return A pointer to the new collection or NULL on failure
 */
struct am_buffered_event_collection*
am_buffered_trace_new_collection(struct am_buffered_trace* bt,
				 size_t buffer_size)
{
	struct am_buffered_event_collection* bec;
	am_event_collection_id_t id;

	id = (bt->num_collections == 0) ? 0 : bt->highest_collection_id + 1;

	if(!(bec = malloc(sizeof(*bec))))
		goto out_err;

	if(am_buffered_event_collection_init(bec, id, buffer_size))
		goto out_err_free;

	if(am_buffered_trace_add_collection(bt, bec))
		goto out_err_destroy;

	return bec;

out_err_destroy:
	am_buffered_event_collection_destroy(bec);
out_err_free:
	free(bec);
out_err:
	return NULL;
}

/**
 * Adds the hierarchy h to the list of event hierarchies of bt.
 *
 * @return 0 on success, otherwise 1.
 */
static inline int am_buffered_trace_add_hierarchy(struct am_buffered_trace* bt,
						  struct am_simple_hierarchy* h)
{
	void* tmp;

	if(h->id <= bt->highest_hierarchy_id)
		for(size_t i = 0; i < bt->num_hierarchies; i++)
			if(bt->hierarchies[i]->id == h->id)
				return 1;

	if(!(tmp = am_grow_array_safe(bt->hierarchies,
				      bt->num_hierarchies,
				      1,
				      sizeof(bt->hierarchies[0]))))
	{
		return 1;
	}

	bt->hierarchies = tmp;
	bt->hierarchies[bt->num_hierarchies] = h;
	bt->num_hierarchies++;

	if(h->id > bt->highest_hierarchy_id)
		bt->highest_hierarchy_id = h->id;

	return 0;
}

/**
 * Adds a new hierarchy to the trace, created from the hierarchy specification
 * given in spec (@see am_simple_hierarchy_build for an explanation of the
 * specification)
 *
 * @return a pointer to the newly created heirarchy or NULL on failure
 */
struct am_simple_hierarchy*
am_buffered_trace_new_hierarchy(struct am_buffered_trace* bt,
				const char* name,
				const char* spec)
{
	struct am_simple_hierarchy* h;
	am_hierarchy_id_t id;

	id = (bt->num_hierarchies == 0) ? 0 : bt->highest_hierarchy_id + 1;

	if(!(h = am_simple_hierarchy_build(name, id, spec)))
		goto out_err;

	if(am_buffered_trace_add_hierarchy(bt, h))
		goto out_err_destroy;

	return h;

out_err_destroy:
	am_simple_hierarchy_destroy(h);
	free(h);
out_err:
	return NULL;
}

/**
 * Retrieves a hierarchy with a specific name from the list of hierarchies
 * associated to the trace. If no such hierarchy exists, NULL is returned.
 */
struct am_simple_hierarchy*
am_buffered_trace_get_hierarchy(struct am_buffered_trace* bt,
				const char* name)
{
	for(size_t i = 0; i < bt->num_hierarchies; i++)
		if(strcmp(bt->hierarchies[i]->name, name) == 0)
			return bt->hierarchies[i];

	return NULL;
}
