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

#include <aftermath/core/dfg_buffer.h>
#include <aftermath/core/safe_alloc.h>
#include <string.h>
#include <stdint.h>

/*
 * Initialize an empty buffer of type sample_type
 */
void am_dfg_buffer_init(struct am_dfg_buffer* b,
			const struct am_dfg_type* sample_type)
{
	b->data = NULL;
	b->num_samples = 0;
	b->num_refs = 0;
	b->max_samples = 0;
	b->sample_type = sample_type;

	INIT_LIST_HEAD(&b->list);
}

/* Resize a buffer such that a total of num_samples samples can be stored in
 * it. Does not invoke any constructor for samples that are allocated, but calls
 * the destructor for samples that are freed.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_buffer_resize(struct am_dfg_buffer* b, size_t new_num_samples)
{
	void* tmp;
	void* destroy_start;
	const struct am_dfg_type* t = b->sample_type;

	/* If we're shrinking: invoke destructors for elements that will be
	 * lost */
	if(new_num_samples < b->num_samples && t->destroy_samples) {
		if(!(destroy_start = am_array_element_ptr_safe(b->data,
							       b->num_samples,
							       t->sample_size)))
		{
			return 1;
		}

		t->destroy_samples(b->sample_type,
				   b->num_samples - new_num_samples,
				   destroy_start);

		/* Already assign new number of samples to leave buffer in
		 * correct state even if shrinking below fails */
		b->num_samples = new_num_samples;
	}

	if(new_num_samples != b->max_samples) {
		/* Need to treat 0 samples separately, since realloc() used by
		 * am_realloc_array_safe() might return a NULL pointer for
		 * zero-byte allocations, which shouldn't be considered an
		 * error. */
		if(new_num_samples != 0) {
			if(!(tmp = am_realloc_array_safe(
				     b->data, new_num_samples,
				     b->sample_type->sample_size)))
			{
				return 1;
			}

			b->data = tmp;
		} else {
			free(b->data);
			b->data = NULL;
		}

		b->max_samples = new_num_samples;
	}

	return 0;
}

/* Reserves space for num_samples samples and returns a pointer to the first of
 * the reserved samples. If the reservation fails, NULL is returned. The number
 * of samples in the buffer is increased by num_samples.
 */
void* am_dfg_buffer_reserve(struct am_dfg_buffer* b, size_t num_samples)
{
	size_t new_size;
	size_t old_size = b->num_samples;
	void* ret;

	/* Overflow of size_t? */
	if(am_size_add_safe(&new_size, b->num_samples, num_samples))
		return NULL;

	/* Resize necessary? */
	if(new_size > b->max_samples)
		if(am_dfg_buffer_resize(b, new_size))
			return NULL;

	ret = am_array_element_ptr_safe(b->data, old_size,
					b->sample_type->sample_size);

	if(!ret)
		return NULL;

	b->num_samples = new_size;

	return ret;
}

/* Shrinks a buffer by num_samples samples.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_buffer_shrink(struct am_dfg_buffer* b, size_t num_samples)
{
	if(b->num_samples < num_samples)
		return 1;

	return am_dfg_buffer_resize(b, b->num_samples - num_samples);
}

/*
 * Append num_saples samples of data to a buffer. The buffer is dynamically
 * resized if its capacity is insufficient.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_buffer_write(struct am_dfg_buffer* b, size_t num_samples, void* data)
{
	void* dst;
	size_t num_bytes;

	if(am_size_mul_safe(&num_bytes, num_samples, b->sample_type->sample_size))
		return 1;

	if(!(dst = am_dfg_buffer_reserve(b, num_samples)))
		return 1;

	memcpy(dst, data, num_bytes);

	return 0;
}

/* Retrieve num_samples samples starting at the offset sample_offset (in
 * samples). If the number of samples is less than the requested number of
 * samples behind the offset, the function returns 1, otherwise 0. */
int am_dfg_buffer_get(struct am_dfg_buffer* b,
		   size_t sample_offset,
		   size_t num_samples,
		   void* data)
{
	void* startaddr;
	size_t num_bytes;

	/* Offset already past the end? */
	if(sample_offset > b->num_samples)
		return 1;

	if(!(startaddr = am_array_element_ptr_safe(b->data, sample_offset,
						   b->sample_type->sample_size)))
	{
		return 1;
	}

	/* Not reading past the end? */
	if(b->num_samples - sample_offset < num_samples)
		return 1;

	if(am_size_mul_safe(&num_bytes, num_samples, b->sample_type->sample_size))
		return 1;

	memcpy(data, startaddr, num_bytes);

	return 0;
}

/*
 * Read num_saples of data from a buffer. The data read from the buffer is not
 * removed and remains accessible afterwards. Hence, subsequent calls to
 * am_dfg_buffer_read return the same data. If the number of samples is less than
 * the requested number the function returns 1, otherwise 0.
 */
int am_dfg_buffer_read(struct am_dfg_buffer* b, size_t num_samples, void* data)
{
	return am_dfg_buffer_get(b, 0, num_samples, data);
}

/* Reads the last sample from a buffer. The data read from the buffer is not
 * removed and remains accessible afterwards. Hence, subsequent calls to
 * am_dfg_buffer_read return the same data. If no sample is present in the
 * buffer, the function returns 1, otherwise 0.
 *
 */
int am_dfg_buffer_read_last(struct am_dfg_buffer* b, void* data)
{
	if(b->num_samples == 0)
		return 1;

	return am_dfg_buffer_get(b, b->num_samples - 1, 1, data);
}

/*
 * Reset the number of samples of a buffer without resizing it (its capacity is
 * preserved).
 */
void am_dfg_buffer_reset(struct am_dfg_buffer* b)
{
	if(b->num_samples != 0 && b->sample_type->destroy_samples) {
		b->sample_type->destroy_samples(b->sample_type,
						b->num_samples,
						b->data);
	}

	b->num_samples = 0;
}

/*
 * Destroy a buffer, including all data held by the buffer
 */
void am_dfg_buffer_destroy(struct am_dfg_buffer* b)
{
	am_dfg_buffer_reset(b);
	free(b->data);
}

/* Increase the number of references to the buffer by one */
void am_dfg_buffer_inc_ref(struct am_dfg_buffer* b)
{
	b->num_refs++;
}

/* Decrease the number of references to the buffer by one */
void am_dfg_buffer_dec_ref(struct am_dfg_buffer* b)
{
	b->num_refs--;
}
