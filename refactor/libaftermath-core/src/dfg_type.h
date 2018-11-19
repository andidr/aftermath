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

#ifndef AM_DFG_TYPES_H
#define AM_DFG_TYPES_H

#include <aftermath/core/contrib/linux-kernel/list.h>
#include <stdlib.h>

struct am_dfg_type;

typedef void (*am_dfg_type_destroy_samples_fun_t)(const struct am_dfg_type* t,
						  size_t num_samples,
						  void* ptr);

typedef int (*am_dfg_type_to_string_fun_t)(const struct am_dfg_type* t,
					   void* ptr, char** out, int* cst);

typedef int (*am_dfg_type_from_string_fun_t)(const struct am_dfg_type* t,
					     const char* str, void* out);

typedef int (*am_dfg_type_check_string_fun_t)(const struct am_dfg_type* t,
					      const char* str);

/* Represents a data type (e.g., for ports) */
struct am_dfg_type {
	/* Name of the type */
	char* name;

	/* Chaining in a type registry */
	struct list_head list;

	/* Size of a sample of this type */
	size_t sample_size;

	/* Function called when an array of sample of this type is to be
	 * destroyed (e.g., a destructor that frees memory not exposed to the
	 * system through sample_size). Can be null if no destruction is needed
	 * prior to freeing the memory. The argument ptr is a pointer to the
	 * first sample and must not be freed. */
	am_dfg_type_destroy_samples_fun_t destroy_samples;

	/* Function converting a single sample of the type into a
	 * string. Ownership of the returned string is transferred to the
	 * caller, unless the output parameter cst is set to 1. A return value
	 * of 0 indicates a successful conversion, whereas 1 indicates an
	 * error. */
	am_dfg_type_to_string_fun_t to_string;

	/* Function converting a string into a single sample of the type. Memory
	 * for the sample is allocated by the caller prior to the call and the
	 * owner of the object remains the caller. A return value of 0 indicates
	 * a successful conversion, whereas 1 indicates an error. */
	am_dfg_type_from_string_fun_t from_string;

	/* Checks if a string is valid and could be passed to
	 * from_string(). Returns 1 if the check passes, otherwise 0. */
	am_dfg_type_check_string_fun_t check_string;
};

int am_dfg_type_init(struct am_dfg_type* t, const char* name, size_t sample_size);
void am_dfg_type_destroy(struct am_dfg_type* t);

/* Single static definition of a builtin type */
struct am_dfg_static_type_def {
	/* Type name */
	const char* name;

	/* Size in bytes of one sample of the type */
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

/* When we're not using the definitions, i.e., in all code outside of the
 * translation unit defining the builtin types, expand type declarations to
 * no-ops. */
#ifndef AM_DFG_GEN_BUILTIN_TYPES
	#define AM_DFG_DECL_BUILTIN_TYPE_SWITCH(...)
	#define AM_DFG_ADD_BUILTIN_TYPES_SWITCH(...)
#endif

/* Generates a static definition of a DFG type.
 *
 * ID must be a globally unique identifier for the type.
 *
 * NAME must be a string identifying the type, e.g., "am::core::string"
 *
 * INSTANCE_SIZE must be a size_t indicating the size of a single instance in
 * bytes
 *
 * ARRAY_DESTRUCTOR_FUN refers to a function of type
 * am_dfg_type_destroy_samples_fun_t that destroys an array of instances, but
 * does not free the array itself; may be NULL if no destructor is needed
 *
 * TO_STRING_FUN is a function of type am_dfg_type_to_string_fun_t, converting a
 * single instance of the type to string; may be NULL.
 *
 * FROM_STRING_FUN is a function of type am_dfg_type_from_string_fun_t,
 * constructing a single instance of the type from a string; may be NULL.
 *
 * CHECK_STRING_FUN is a function of type am_dfg_type_check_string_fun_t,
 * checking if a string is a valid representation of an instance of the type;
 * may be NULL.
 *
 * Example:
 *
 *   AM_DFG_DECL_BUILTIN_TYPE(am_dfg_type_bool,
 *   	"am::core::bool",
 *   	AM_DFG_TYPE_BOOL_SAMPLE_SIZE,
 *   	NULL,
 *   	am_dfg_type_bool_to_string,
 *   	am_dfg_type_bool_from_string,
 *   	am_dfg_type_bool_check_string)
 */
#define AM_DFG_DECL_BUILTIN_TYPE(ID, NAME, INSTANCE_SIZE, ARRAY_DESTRUCTOR_FUN, \
				 TO_STRING_FUN, FROM_STRING_FUN,		\
				 CHECK_STRING_FUN)				\
	AM_DFG_DECL_BUILTIN_TYPE_SWITCH(ID, NAME, INSTANCE_SIZE,		\
					ARRAY_DESTRUCTOR_FUN,			\
					TO_STRING_FUN, FROM_STRING_FUN,	\
					CHECK_STRING_FUN)

/* Adds the types associated to the identifiers passed as arguments to the list
 * of builtin DFG types. There can only be one invocation of
 * AM_DFG_ADD_BUILTIN_TYPES per header file.
 *
 * Example:
 *
 *   AM_DFG_DECL_BUILTIN_TYPE(am_dfg_type_string, ...)
 *   AM_DFG_DECL_BUILTIN_TYPE(am_dfg_type_interval, ...)
 *   AM_DFG_DECL_BUILTIN_TYPE(am_dfg_type_duration, ...)
 *
 *   AM_DFG_ADD_BUILTIN_TYPES(am_dfg_type_string, .
 *			      am_dfg_type_interval,
 *			      am_dfg_type_duration)
 */
#define AM_DFG_ADD_BUILTIN_TYPES(...) \
	AM_DFG_ADD_BUILTIN_TYPES_SWITCH(__VA_ARGS__)

#endif
