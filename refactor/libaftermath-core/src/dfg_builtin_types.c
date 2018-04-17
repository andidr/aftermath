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
#include <aftermath/core/dfg/types/bool.h>
#include <aftermath/core/dfg/types/duration.h>
#include <aftermath/core/dfg/types/generic.h>
#include <aftermath/core/dfg/types/histogram.h>
#include <aftermath/core/dfg/types/histogram_data.h>
#include <aftermath/core/dfg/types/interval.h>
#include <aftermath/core/dfg/types/string.h>
#include <aftermath/core/dfg/types/timestamp.h>
#include <aftermath/core/dfg/types/trace.h>
#include <aftermath/core/dfg/types/int.h>

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

	/* Conversion from string */
	am_dfg_type_from_string_fun_t from_string;

	/* Checks if string is valid to be passed to from_string */
	am_dfg_type_check_string_fun_t check_string;
};

#define AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(SUFFIX, SUFFIX_UPCASE)		\
	{ "am::core::histogram1d<" #SUFFIX ">",				\
			AM_DFG_TYPE_HISTOGRAM1D_##SUFFIX_UPCASE##_SAMPLE_SIZE,	\
			am_dfg_type_histogram1d_##SUFFIX##_free_samples,	\
			NULL,							\
			NULL,							\
			NULL }

#define AM_DFG_INT_TYPE_STATIC_DECL(TPREFIX, TPREFIXUP)	\
	{ "am::core::" #TPREFIX,				\
	  AM_DFG_TYPE_##TPREFIXUP##_SAMPLE_SIZE,		\
	  NULL,						\
	  am_dfg_type_##TPREFIX##_to_string,			\
	  am_dfg_type_##TPREFIX##_from_string,			\
	  am_dfg_type_##TPREFIX##_check_string }

static struct static_dfg_type_decl types[] = {
	{ "am::core::bool",
	  AM_DFG_TYPE_BOOL_SAMPLE_SIZE,
	  NULL,
	  am_dfg_type_bool_to_string,
	  am_dfg_type_bool_from_string,
	  am_dfg_type_bool_check_string },
	{ "am::core::timestamp",
	  AM_DFG_TYPE_TIMESTAMP_SAMPLE_SIZE,
	  NULL,
	  am_dfg_type_timestamp_to_string,
	  am_dfg_type_timestamp_from_string,
	  am_dfg_type_timestamp_check_string },
	{ "const am::core::trace",
	  AM_DFG_TYPE_TRACE_SAMPLE_SIZE,
	  NULL },
	{ "am::core::duration",
	  AM_DFG_TYPE_DURATION_SAMPLE_SIZE,
	  NULL,
	  am_dfg_type_duration_to_string,
	  am_dfg_type_duration_from_string,
	  am_dfg_type_duration_check_string },
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint8, UINT8),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint16, UINT16),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint32, UINT32),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint64, UINT64),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int8, INT8),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int16, INT16),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int32, INT32),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int64, INT64),
	AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(double, DOUBLE),
	{ "const am::core::event_mapping",
	  sizeof(struct am_event_mapping*),
	  NULL },
	{ "const am::core::hierarchy",
	  sizeof(struct am_event_hierarchy*),
	  NULL },
	{ "am::core::histogram1d_data",
	  AM_DFG_TYPE_HISTOGRAM1D_DATA_SAMPLE_SIZE,
	  am_dfg_type_histogram1d_data_free_samples },
	{ "am::core::interval",
	  AM_DFG_TYPE_INTERVAL_SAMPLE_SIZE,
	  NULL,
	  am_dfg_type_interval_to_string,
	  am_dfg_type_interval_from_string,
	  am_dfg_type_interval_check_string },
	{ "am::core::string",
	  AM_DFG_TYPE_STRING_SAMPLE_SIZE,
	  am_dfg_type_generic_free_samples,
	  am_dfg_type_string_to_string,
	  am_dfg_type_string_from_string,
	  am_dfg_type_string_check_string },
	{ "am::core::state_event",
	  sizeof(struct am_state_event),
	  NULL },
	AM_DFG_INT_TYPE_STATIC_DECL( int8,  INT8),
	AM_DFG_INT_TYPE_STATIC_DECL(int16, INT16),
	AM_DFG_INT_TYPE_STATIC_DECL(int32, INT32),
	AM_DFG_INT_TYPE_STATIC_DECL(int64, INT64),
	AM_DFG_INT_TYPE_STATIC_DECL( uint8,  UINT8),
	AM_DFG_INT_TYPE_STATIC_DECL(uint16, UINT16),
	AM_DFG_INT_TYPE_STATIC_DECL(uint32, UINT32),
	AM_DFG_INT_TYPE_STATIC_DECL(uint64, UINT64),
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
		t->from_string = std->from_string;
		t->check_string = std->check_string;

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
