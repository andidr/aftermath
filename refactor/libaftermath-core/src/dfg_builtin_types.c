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

#include <aftermath/core/dfg_builtin_types.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/in_memory.h>
#include <stdio.h>

/* Default destructor for pointer samples (e.g., char* aka string) */
static void free_ptr_samples(const struct am_dfg_type* t,
			     size_t num_samples,
			     void* ptr)
{
	void** pptr = ptr;
	void* curr;

	for(size_t i = 0; i < num_samples; i++) {
		curr = pptr[i];
		free(curr);
	}
}

static int string_to_string(const struct am_dfg_type* t,
			    void* ptr,
			    char** out,
			    int* cst)
{
	*cst = 1;
	*out = ptr;

	return 0;
}

static int timestamp_to_string(const struct am_dfg_type* t,
			       void* ptr,
			       char** out,
			       int* cst)
{
	char* ret;
	am_timestamp_t* ts = ptr;

	if(!(ret = malloc(AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 1)))
		return 1;

	snprintf(ret, AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS,
		 "%" AM_TIMESTAMP_T_FMT, *ts);

	*out = ret;

	return 0;
}

static int duration_to_string(const struct am_dfg_type* t,
			       void* ptr,
			       char** out,
			       int* cst)
{
	char* ret;
	struct am_time_offset* d = ptr;

	if(!(ret = malloc(AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 2)))
		return 1;

	snprintf(ret, AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS,
		 "%s%" AM_TIMESTAMP_T_FMT,
		 (d->sign) ? "-" : "",
		 d->abs);

	*out = ret;

	return 0;
}

static int interval_to_string(const struct am_dfg_type* t,
			      void* ptr,
			      char** out,
			      int* cst)
{
	char* ret;
	struct am_interval* i = ptr;

	if(!(ret = malloc(2*AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 5)))
		return 1;

	snprintf(ret, 2 * AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 4,
		 "[%" AM_TIMESTAMP_T_FMT ", %" AM_TIMESTAMP_T_FMT "]",
		 i->start, i->end);

	*out = ret;

	return 0;
}

struct static_dfg_type_decl {
	/* Type name */
	const char* name;

	/* Size in bytes of one instance of the type */
	size_t sample_size;

	/* Function for the destruction of samples. May be NULL if no
	 * destruction is needed. */
	am_dfg_type_destroy_samples_fun_t destroy_samples;

	/* Conversion to string */
	am_dfg_type_to_string_fun_t to_string;
};

static struct static_dfg_type_decl types[] = {
	{ "timestamp", sizeof(am_timestamp_t), NULL, timestamp_to_string },
	{ "duration", sizeof(struct am_time_offset), NULL, duration_to_string },
	{ "interval", sizeof(struct am_interval), NULL, interval_to_string },
	{ "string", sizeof(char*), free_ptr_samples, string_to_string },
	{ NULL } /* End marker */
};

/* Destroys a list of types */
static void am_dfg_builtin_type_list_destroy(struct list_head* list)
{
	struct am_dfg_type* t;
	struct am_dfg_type* next;

	am_typed_list_for_each_safe_genentry(list, t, next, list) {
		am_dfg_type_destroy(t);
		free(t);
	}
}

/* Registers the array types of static DFG type declarations at the registry
 * tr. Returns 0 on success, otherwise 1. The pointer "name" of the last entry
 * of the array must be NULL. */
static int
am_dfg_builtin_types_register_static_decls(struct am_dfg_type_registry* tr,
					   struct static_dfg_type_decl* types)
{
	struct list_head list;
	struct static_dfg_type_decl* std;
	struct am_dfg_type* t;
	struct am_dfg_type* next;

	INIT_LIST_HEAD(&list);

	/* First reserve memory for each type and initialize */
	for(std = types; std->name; std++) {
		if(!(t = malloc(sizeof(*t))))
			goto out_err_t;

		if(am_dfg_type_init(t, std->name, std->sample_size))
			goto out_err;

		t->destroy_samples = std->destroy_samples;
		t->to_string = std->to_string;

		list_add(&t->list, &list);
	}

	/* Register entire list of initialized types. Use safe version for
	 * iteration, since the embedded list of the type will be used to
	 * enqueue the type at the type registry.  */
	am_typed_list_for_each_safe_genentry(&list, t, next, list)
		am_dfg_type_registry_add(tr, t);

	return 0;

out_err_t:
	free(t);
out_err:
	am_dfg_builtin_type_list_destroy(&list);
	return 1;
}

/* Registers the all builtin DFG types at the registry tr. Returns 0 on success,
 * otherwise 1. */
int am_dfg_builtin_types_register(struct am_dfg_type_registry* tr)
{
	return am_dfg_builtin_types_register_static_decls(tr, types);
}
