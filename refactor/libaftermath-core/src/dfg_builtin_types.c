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
#include <aftermath/core/dfg_builtin_type_impl.h>
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

#define AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(SUFFIX, SUFFIX_UPCASE)	\
	AM_DFG_DECL_BUILTIN_TYPE(					\
		am_dfg_type_histogram1d_##SUFFIX,			\
		"am::core::histogram1d<" #SUFFIX ">",			\
		AM_DFG_TYPE_HISTOGRAM1D_##SUFFIX_UPCASE##_SAMPLE_SIZE,	\
		am_dfg_type_histogram1d_##SUFFIX##_free_samples,	\
		NULL,							\
		NULL,							\
		NULL)

#define AM_DFG_INT_TYPE_STATIC_DECL(TPREFIX, TPREFIXUP)	\
	AM_DFG_DECL_BUILTIN_TYPE(				\
		am_dfg_type_##TPREFIX,				\
		"am::core::" #TPREFIX,				\
		AM_DFG_TYPE_##TPREFIXUP##_SAMPLE_SIZE,		\
		NULL,						\
		am_dfg_type_##TPREFIX##_to_string,		\
		am_dfg_type_##TPREFIX##_from_string,		\
		am_dfg_type_##TPREFIX##_check_string)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_bool,
	"am::core::bool",
	AM_DFG_TYPE_BOOL_SAMPLE_SIZE,
	NULL,
	am_dfg_type_bool_to_string,
	am_dfg_type_bool_from_string,
	am_dfg_type_bool_check_string)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_timestamp,
	"am::core::timestamp",
	AM_DFG_TYPE_TIMESTAMP_SAMPLE_SIZE,
	NULL,
	am_dfg_type_timestamp_to_string,
	am_dfg_type_timestamp_from_string,
	am_dfg_type_timestamp_check_string)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_trace,
	"const am::core::trace",
	AM_DFG_TYPE_TRACE_SAMPLE_SIZE,
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_duration,
	"am::core::duration",
	AM_DFG_TYPE_DURATION_SAMPLE_SIZE,
	NULL,
	am_dfg_type_duration_to_string,
	am_dfg_type_duration_from_string,
	am_dfg_type_duration_check_string)


AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint8, UINT8)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint16, UINT16)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint32, UINT32)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint64, UINT64)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int8, INT8)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int16, INT16)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int32, INT32)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int64, INT64)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(double, DOUBLE)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_event_mapping,
	"const am::core::event_mapping",
	sizeof(struct am_event_mapping*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_hierarchy,
	"const am::core::hierarchy",
	sizeof(struct am_event_hierarchy*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_hierarchy_node,
	"const am::core::hierarchy_node",
	sizeof(struct am_event_hierarchy_node*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_histogram1d_data,
	"am::core::histogram1d_data",
	AM_DFG_TYPE_HISTOGRAM1D_DATA_SAMPLE_SIZE,
	am_dfg_type_histogram1d_data_free_samples,
	NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_interval,
	"am::core::interval",
	AM_DFG_TYPE_INTERVAL_SAMPLE_SIZE,
	NULL,
	am_dfg_type_interval_to_string,
	am_dfg_type_interval_from_string,
	am_dfg_type_interval_check_string)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_string,
	"am::core::string",
	AM_DFG_TYPE_STRING_SAMPLE_SIZE,
	am_dfg_type_generic_free_samples,
	am_dfg_type_string_to_string,
	am_dfg_type_string_from_string,
	am_dfg_type_string_check_string)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_state_event,
	"am::core::state_event",
	sizeof(struct am_state_event),
	NULL, NULL, NULL, NULL)

AM_DFG_INT_TYPE_STATIC_DECL( int8,  INT8)
AM_DFG_INT_TYPE_STATIC_DECL(int16, INT16)
AM_DFG_INT_TYPE_STATIC_DECL(int32, INT32)
AM_DFG_INT_TYPE_STATIC_DECL(int64, INT64)
AM_DFG_INT_TYPE_STATIC_DECL( uint8,  UINT8)
AM_DFG_INT_TYPE_STATIC_DECL(uint16, UINT16)
AM_DFG_INT_TYPE_STATIC_DECL(uint32, UINT32)
AM_DFG_INT_TYPE_STATIC_DECL(uint64, UINT64)

#define DEFS_NAME() am_dfg_builtin_types

AM_DFG_ADD_BUILTIN_TYPES(
	&am_dfg_type_bool,
	&am_dfg_type_timestamp,
	&am_dfg_type_const_trace,
	&am_dfg_type_duration,
	&am_dfg_type_histogram1d_uint8,
	&am_dfg_type_histogram1d_uint16,
	&am_dfg_type_histogram1d_uint32,
	&am_dfg_type_histogram1d_uint64,
	&am_dfg_type_histogram1d_int8,
	&am_dfg_type_histogram1d_int16,
	&am_dfg_type_histogram1d_int32,
	&am_dfg_type_histogram1d_int64,
	&am_dfg_type_histogram1d_double,

	&am_dfg_type_histogram1d_data,

	&am_dfg_type_const_event_mapping,
	&am_dfg_type_const_hierarchy,
	&am_dfg_type_const_hierarchy_node,

	&am_dfg_type_interval,
	&am_dfg_type_string,
	&am_dfg_type_state_event,

	&am_dfg_type_int8,
	&am_dfg_type_int16,
	&am_dfg_type_int32,
	&am_dfg_type_int64,
	&am_dfg_type_uint8,
	&am_dfg_type_uint16,
	&am_dfg_type_uint32,
	&am_dfg_type_uint64,
	NULL
)

static struct am_dfg_static_type_def** defsets[] = {
	am_dfg_builtin_types,
	NULL
};

/* Registers the all builtin DFG types at the registry tr. Returns 0 on success,
 * otherwise 1. */
int am_dfg_builtin_types_register(struct am_dfg_type_registry* tr)
{
	return am_dfg_type_registry_add_static(tr, defsets);
}
