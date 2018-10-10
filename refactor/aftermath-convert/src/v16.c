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

#include "v16.h"
#include "v16_structs.h"
#include <aftermath/core/convert.h>
#include <aftermath/core/io_context.h>
#include <aftermath/core/on_disk.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/safe_alloc.h>
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/bits.h>
#include <stdint.h>

/* The old trace format had a set of built-in states that didn't require a state
 * description; define the list of built-in states */
static char* v16_default_state_names[] = {
	"seeking",
	"taskexec",
	"tcreate",
	"resdep",
	"tdec",
	"bcast",
	"init",
	"estimate_costs",
	"reorder"
};

#define V16_NUM_DEFAULT_STATES AM_ARRAY_SIZE(v16_default_state_names)

/* Structure accumulating information on a CPU: CPU number discovered through
 * the CPU field from the events, the NUMA node if define by a cpu info and the
 * ID of default hierarchy node in the output trace associated to the CPU's
 * event collection */
struct v16_cpu_def {
	uint32_t cpu;
	uint32_t numa_node;
	int numa_node_known;
	uint32_t hnode_id;
};

#define ACC_CPU(cpu_def) (cpu_def).cpu

AM_DECL_TYPED_ARRAY(v16_cpu_array, struct v16_cpu_def)
AM_DECL_TYPED_ARRAY_BSEARCH(v16_cpu_array, struct v16_cpu_def,
			    uint32_t, ACC_CPU, AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_INSERTPOS(v16_cpu_array, struct v16_cpu_def,
			    uint32_t, ACC_CPU, AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(v16_cpu_array, struct v16_cpu_def, uint32_t)

/* Mapping of a NUMA node ID as discovered in a cpu info event of the input
 * trace to the default hierarchy node in the output trace */
struct v16_numa_node_def {
	uint32_t id;
	uint32_t hnode_id;
};

#define ACC_NUMA_NODE(numa_node_def) ((numa_node_def).id)

AM_DECL_TYPED_ARRAY(v16_numa_node_array, struct v16_numa_node_def)
AM_DECL_TYPED_ARRAY_BSEARCH(v16_numa_node_array, struct v16_numa_node_def,
			    uint32_t, ACC_NUMA_NODE, AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_INSERTPOS(v16_numa_node_array, struct v16_numa_node_def,
			      uint32_t, ACC_NUMA_NODE, AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(v16_numa_node_array, struct v16_numa_node_def,
				   uint32_t)

/* Data structure capturing the necessary data of a texec start event used when
 * processing the corresponding texec end event. */
struct v16_texec_start {
	uint32_t cpu;
	uint64_t workfn;
	uint64_t time;
	int valid;
};

/* Initializes a texec start data structure from a single event defining a texec
 * start event. */
static inline void v16_texec_start_init(
	struct v16_texec_start* me,
	struct v16_trace_single_event* sge)
{
	me->valid = 0;
	me->cpu = sge->header.cpu;
	me->workfn = sge->what;
	me->time = sge->header.time;
}

AM_DECL_TYPED_ARRAY(v16_texec_start_array, struct v16_texec_start)
AM_DECL_TYPED_ARRAY_BSEARCH(v16_texec_start_array,
			    struct v16_texec_start,
			    uint32_t,
			    ACC_CPU,
			    AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_INSERTPOS(v16_texec_start_array,
			      struct v16_texec_start,
			      uint32_t,
			      ACC_CPU,
			      AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(v16_texec_start_array,
				   struct v16_texec_start,
				   uint32_t)

#define ACC_IDENT(x) (x)

AM_DECL_TYPED_ARRAY(u64_array, uint64_t)
AM_DECL_TYPED_ARRAY_BSEARCH(u64_array, uint64_t, uint64_t, ACC_IDENT,
			    AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_INSERTPOS(u64_array, uint64_t, uint64_t, ACC_IDENT,
			      AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(u64_array, uint64_t, uint64_t)

/* Conversion context for conversion from format version 16 to the current trace
 * format */
struct v16ctx {
	/* Output I/O context for generation of the output trace */
	struct am_io_context octx;

	/* Array of CPUs encountered in the input trace */
	struct v16_cpu_array cpus;

	/* Array of NUMA nodes encountered in the input trace */
	struct v16_numa_node_array numa_nodes;

	/* Error stack used for error reporting */
	struct am_io_error_stack* estack;

	/* Bitmap with IDs of built-in states for which no state description has
	 * been written */
	uint64_t default_states_missing_description;

	/* Maximum timestamp discovered in the input file */
	uint64_t max_timestamp;

	/* Input file in v16 format */
	FILE* fp_in;

	/* In the v16, measurement intervals are split across two global single
	 * events. This interval collects the start and end timestamp of the two
	 * events. */
	struct am_interval measurement_interval;

	/* If one, a global single event starting a measurement interval has
	 * been encountered, but not yet the end interval. */
	int measurement_interval_started;

	struct {
		/* Array with (at most) one entry per CPU indicating the last
		 * encountered texec start event */
		struct v16_texec_start_array last_texec_start;
	} per_cpu;

	/* Adresses of work functions for which an OpenStream task type frame
	 * has been written */
	struct u64_array openstream_task_type_addresses;

	/* Current ID used for ID generation */
	uint64_t curr_id;

	/* Adresses of work functions for which an OpenMP task type frame
	 * has been written */
	struct u64_array omp_task_type_addresses;
};

/* IDs used for frame types in the output file */
enum output_frame_type {
	AM_FRAME_TYPE_FRAME_TYPE_ID = 0,
	AM_FRAME_TYPE_EVENT_COLLECTION,
	AM_FRAME_TYPE_HIERARCHY_DESCRIPTION,
	AM_FRAME_TYPE_HIERARCHY_NODE,
	AM_FRAME_TYPE_STATE_DESCRIPTION,
	AM_FRAME_TYPE_STATE_EVENT,
	AM_FRAME_TYPE_COUNTER_DESCRIPTION,
	AM_FRAME_TYPE_COUNTER_EVENT,
	AM_FRAME_TYPE_MEASUREMENT_INTERVAL,
	AM_FRAME_TYPE_EVENT_MAPPING,
	AM_FRAME_TYPE_OPENSTREAM_TASK_TYPE,
	AM_FRAME_TYPE_OPENSTREAM_TASK_INSTANCE,
	AM_FRAME_TYPE_OPENSTREAM_TASK_PERIOD,
	AM_FRAME_TYPE_OPENMP_TASK_TYPE,
	AM_FRAME_TYPE_OPENMP_TASK_INSTANCE,
	AM_FRAME_TYPE_OPENMP_TASK_PERIOD,
	AM_FRAME_TYPE_NUM
};

/* Mapping from IDs to types in the output file */
static struct {
	uint32_t id;
	char* name;
} frame_id_to_name[AM_FRAME_TYPE_NUM] = {
	{ AM_FRAME_TYPE_EVENT_COLLECTION, "am_dsk_event_collection" },
	{ AM_FRAME_TYPE_HIERARCHY_DESCRIPTION, "am_dsk_hierarchy_description" },
	{ AM_FRAME_TYPE_HIERARCHY_NODE, "am_dsk_hierarchy_node" },
	{ AM_FRAME_TYPE_STATE_DESCRIPTION, "am_dsk_state_description" },
	{ AM_FRAME_TYPE_STATE_EVENT, "am_dsk_state_event" },
	{ AM_FRAME_TYPE_COUNTER_DESCRIPTION, "am_dsk_counter_description" },
	{ AM_FRAME_TYPE_COUNTER_EVENT, "am_dsk_counter_event" },
	{ AM_FRAME_TYPE_MEASUREMENT_INTERVAL, "am_dsk_measurement_interval" },
	{ AM_FRAME_TYPE_EVENT_MAPPING, "am_dsk_event_mapping" },
	{ AM_FRAME_TYPE_OPENSTREAM_TASK_TYPE, "am_dsk_openstream_task_type" },
	{ AM_FRAME_TYPE_OPENSTREAM_TASK_INSTANCE, "am_dsk_openstream_task_instance" },
	{ AM_FRAME_TYPE_OPENSTREAM_TASK_PERIOD, "am_dsk_openstream_task_period" },
	{ AM_FRAME_TYPE_OPENMP_TASK_TYPE, "am_dsk_openmp_task_type" },
	{ AM_FRAME_TYPE_OPENMP_TASK_INSTANCE, "am_dsk_openmp_task_instance" },
	{ AM_FRAME_TYPE_OPENMP_TASK_PERIOD, "am_dsk_openmp_task_period" }
};

/* Registers the frame types used in the ouput file at the frame type registry
 * of the conversion context.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_register_frame_types(struct v16ctx* v16ctx)
{
	struct am_dsk_frame_type_id id;

	/* 0 is the special ID for frame type ID associations */
	uint32_t type = 0;

	for(size_t i = 1; i < AM_FRAME_TYPE_NUM; i++) {
		id.id = frame_id_to_name[i-1].id;
		id.type_name.str = frame_id_to_name[i-1].name;
		id.type_name.len = strlen(id.type_name.str);

		if(am_dsk_frame_type_id_write(&v16ctx->octx, type, &id)) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_WRITE,
					       "Could not write ID association "
					       "for frame type \"%s\".",
					       frame_id_to_name[i-1].name);
			return 1;
		}
	}

	return 0;
}

/* Initializes a v16 conversion context. The input file pointer must be
 * positioned right after the trace version field and the ouput file pointer
 * must be positioned at the beginning of the output file.
 *
 * Returns 0 on success, otherwise 1. */
static int v16ctx_init(struct v16ctx* v16ctx,
		       FILE* fp_in,
		       FILE* fp_out,
		       struct am_io_error_stack* estack)
{
	v16_cpu_array_init(&v16ctx->cpus);
	v16_numa_node_array_init(&v16ctx->numa_nodes);
	v16_texec_start_array_init(&v16ctx->per_cpu.last_texec_start);
	u64_array_init(&v16ctx->openstream_task_type_addresses);
	u64_array_init(&v16ctx->omp_task_type_addresses);

	v16ctx->fp_in = fp_in;
	v16ctx->estack = estack;
	v16ctx->max_timestamp = 0;
	v16ctx->default_states_missing_description =
		(1 << V16_NUM_DEFAULT_STATES) - 1;
	v16ctx->measurement_interval_started = 0;

	if(am_io_context_init(&v16ctx->octx, NULL))
		return 1;

	v16ctx->octx.fp = fp_out;
	v16ctx->curr_id = 0;

	return 0;
}

/* Destroys a v16 conversion context. The input and output file handles are not
 * closed. */
static void v16ctx_destroy(struct v16ctx* v16ctx)
{
	v16_cpu_array_destroy(&v16ctx->cpus);
	v16_numa_node_array_destroy(&v16ctx->numa_nodes);
	v16_texec_start_array_destroy(&v16ctx->per_cpu.last_texec_start);
	u64_array_destroy(&v16ctx->openstream_task_type_addresses);
	u64_array_destroy(&v16ctx->omp_task_type_addresses);

	v16ctx->octx.fp = NULL;

	am_io_context_destroy(&v16ctx->octx);
}

/* Updates the maximum timestamp discovered in the input file if necessary */
static inline void
v16ctx_check_update_max_timestamp(struct v16ctx* v16ctx, uint64_t max_timestamp)
{
	if(v16ctx->max_timestamp < max_timestamp)
		v16ctx->max_timestamp = max_timestamp;
}

/* Writes the file header to the output file in the current format. Returns 0 on
 * success, otherwise 1. */
static int v16ctx_write_file_header(struct v16ctx* v16ctx)
{
	struct am_dsk_header h;

	h.magic = AM_TRACE_MAGIC;
	h.version = AM_TRACE_VERSION;

	if(am_dsk_header_write(&v16ctx->octx, &h)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write trace header.");
		return 1;
	}

	return 0;
}

/* Writes the event mappings to the output file: one mapping for each CPU,
 * mapping the CPU's hierarchy node to the CPU's event collection for an
 * interval corresponding to the entire trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_write_event_mappings(struct v16ctx* v16ctx)
{
	struct v16_cpu_def* cpu_def;
	struct am_dsk_event_mapping m;
	uint32_t type = AM_FRAME_TYPE_EVENT_MAPPING;

	m.interval.start = 0;
	m.interval.end = v16ctx->max_timestamp;

	for(size_t i = 0; i < v16ctx->cpus.num_elements; i++) {
		cpu_def = &v16ctx->cpus.elements[i];

		m.collection_id = cpu_def->cpu;
		m.hierarchy_id = 0;
		m.node_id = cpu_def->hnode_id;

		if(am_dsk_event_mapping_write(&v16ctx->octx, type, &m)) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_WRITE,
					       "Could not write event mapping "
					       "for CPU %" PRIu32 ".",
					       cpu_def->cpu);
			return 1;
		}
	}

	return 0;
}

/* Writes the hierarchy to the output file. This can either be a "flat"
 * hierarchy composed of a root node with one child per CPU, a two-level
 * hierarchy with a root node, one child node per NUMA node and a set of CPU
 * nodes for each NUMA node or a mix of both depending on the presence of cpu
 * info events in the source trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_write_hierarchy(struct v16ctx* v16ctx)
{
	struct am_dsk_hierarchy_description hd;
	struct am_dsk_hierarchy_node hn;
	char buffer[64];
	uint32_t numa_node_id;
	uint32_t hnode_id;
	uint32_t cpu_id;
	uint32_t num_numa_nodes;
	uint32_t cpu_hnode_shift;
	uint32_t numa_hnode_shift;
	struct v16_cpu_def* cpu_def;
	struct v16_numa_node_def* numa_node_def;
	uint32_t hd_type = AM_FRAME_TYPE_HIERARCHY_DESCRIPTION;
	uint32_t hn_type = AM_FRAME_TYPE_HIERARCHY_NODE;

	hd.id = 0;
	hd.name.str = "HW";
	hd.name.len = 2;

	if(am_dsk_hierarchy_description_write(&v16ctx->octx, hd_type, &hd)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write hierarchy description.");
		return 1;
	}

	/* Create root node with ID 1 */
	hn.hierarchy_id = 0;
	hn.id = 1;
	hn.parent_id = 0;
	hn.name.len = 4;
	hn.name.str = "root";

	if(am_dsk_hierarchy_node_write(&v16ctx->octx, hn_type, &hn)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write hierarchy node with id "
				       "%" AM_HIERARCHY_NODE_ID_T_FMT ".",
				       hn.id);
		return 1;
	}

	/* Conversion from size_t to uint32_t is safe here, since the IDs from
	 * the source trace are specified as uint32_t values */
	num_numa_nodes = v16ctx->numa_nodes.num_elements;

	/* Shift NUMA node IDs by two: ID 0 is reserved and ID 1 is the root
	 * node */
	numa_hnode_shift = 2;

	/* Create one hierarchy node per NUMA node */
	for(size_t i = 0; i < num_numa_nodes; i++) {
		numa_node_def = &v16ctx->numa_nodes.elements[i];
		numa_node_id = numa_node_def->id;

		if(am_add_safe_u32(i, numa_hnode_shift, &hnode_id)) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_WRITE,
					       "Could not generate hierarchy "
					       "node ID NUMA node %" PRIu32 ".",
					       numa_node_id);

			return 1;
		}

		numa_node_def->hnode_id = hnode_id;

		snprintf(buffer, sizeof(buffer), "Node %" PRIu32, numa_node_id);
		hn.id = hnode_id;
		hn.parent_id = 1;
		hn.name.str = buffer;
		hn.name.len = strlen(buffer);

		if(am_dsk_hierarchy_node_write(&v16ctx->octx, hn_type, &hn)) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_OVERFLOW,
					       "Could not write hierarchy node "
					       "for NUMA node %" PRIu32 ".",
					       numa_node_id);
			return 1;
		}
	}

	/* Shift the CPU ids by the number of NUMA nodes + 2 to avoid
	 * collisions between hierarchy nodes for CPUs and those for
	 * NUMA nodes. */
	cpu_hnode_shift = num_numa_nodes;

	if(am_add_safe_u32(num_numa_nodes, 2, &cpu_hnode_shift)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_OVERFLOW,
				       "Too many NUMA nodes.");

		return 1;
	}

	/* Write one hierarchy node per CPU */
	for(size_t i = 0; i < v16ctx->cpus.num_elements; i++) {
		cpu_def = &v16ctx->cpus.elements[i];
		cpu_id = cpu_def->cpu;
		snprintf(buffer, sizeof(buffer), "CPU %" PRIu32, cpu_id);

		if(am_add_safe_u32(i, cpu_hnode_shift, &hnode_id)) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_WRITE,
					       "Could not generate hierarchy "
					       "node ID CPU %" PRIu32 ".",
					       cpu_id);

			return 1;
		}

		hn.id = hnode_id;

		/* Use hierarchy node of the NUMA node if the NUMA node is
		 * known, otherwise make this a direct descendant of the root
		 * node */
		if(cpu_def->numa_node_known) {
			numa_node_def = v16_numa_node_array_bsearch(
				&v16ctx->numa_nodes, cpu_def->numa_node);
			hn.parent_id = numa_node_def->hnode_id;
		} else {
			hn.parent_id = 1;
		}

		hn.name.str = buffer;
		hn.name.len = strlen(buffer);

		cpu_def->hnode_id = hnode_id;

		if(am_dsk_hierarchy_node_write(&v16ctx->octx, hn_type, &hn)) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_WRITE,
					       "Could not write hierarchy node "
					       "for CPU %" PRIu32 ".",
					       cpu_id);
			return 1;
		}
	}

	return 0;
}

/* Writes the frame defining an event collection for the CPU with the given id
 * to the output file.
 *
 * Returns 0 on success, otherwise 1.
 */
int v16ctx_write_event_collection(struct v16ctx* v16ctx, uint32_t id)
{
	struct am_dsk_event_collection ec;
	char buffer[64];
	uint32_t type = AM_FRAME_TYPE_EVENT_COLLECTION;

	ec.id = id;

	snprintf(buffer, sizeof(buffer), "CPU %" PRIu32, id);
	ec.name.str = buffer;
	ec.name.len = strlen(buffer);

	if(am_dsk_event_collection_write(&v16ctx->octx, type, &ec)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write event collection.");
		return 1;
	}

	return 0;
}

/* Adds an entry to the array of work function addresses for which an OpenStream
 * task type frame has already been written. Returns 0 on success, otherwise
 * 1. */
static inline int
v16_openstream_task_type_address_add(struct v16ctx* v16ctx, uint64_t addr)
{
	uint64_t* pval;

	if(!(pval = u64_array_reserve_sorted(&v16ctx->openstream_task_type_addresses,
					     addr)))
	{
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Could not allocate task type address "
				       "array entry for work function "
				       "0x%" PRIx64 ".",
				       addr);

		return 1;
	}

	*pval = addr;

	return 0;
}

/* Writes an OpenStream task type frame if such a frame hasn't already been
 * written previously for the work function with the given address. Returns 0 on
 * success, otherwise 1. */
static inline int
v16_write_openstream_task_type_if_necessary(struct v16ctx* v16ctx, uint64_t addr)
{
	struct am_dsk_openstream_task_type tt;
	char buf[64];
	uint32_t type = AM_FRAME_TYPE_OPENSTREAM_TASK_TYPE;

	if(u64_array_bsearch(&v16ctx->openstream_task_type_addresses, addr))
		return 0;

	snprintf(buf, sizeof(buf), "workfn_0x%" PRIx64, addr);

	/* Write task type. Use address of the work function as type ID */
	tt.type_id = addr;
	tt.name.str = buf;
	tt.name.len = strlen(buf);
	tt.source.file.str = "";
	tt.source.file.len = 0;
	tt.source.line = 0;
	tt.source.character = 0;

	if(am_dsk_openstream_task_type_write(&v16ctx->octx, type, &tt)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write OpenStream task type.");
		return 1;
	}

	if(v16_openstream_task_type_address_add(v16ctx, addr))
		return 1;

	return 0;
}

/* Returns a pointer to the mapping entry for the CPU specified in a single
 * event. If no such entry exsists, a new entry is added and initialized with
 * the data from the single event. Returns a pointer to the entry or NULL on
 * error. */
static inline struct v16_texec_start*
v16_texec_start_array_find_add(struct v16_texec_start_array* m,
				struct v16ctx* v16ctx,
				struct v16_trace_single_event* sge)
{
	struct v16_texec_start* me;
	uint32_t cpu = sge->header.cpu;

	if(!(me = v16_texec_start_array_bsearch(m, cpu))) {
		if(!(me = v16_texec_start_array_reserve_sorted(m, cpu))) {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_ALLOC,
					       "Could not allocate CPU "
					       "to timestamp mapping for CPU "
					       "%" PRIu32 ".", cpu);

			return NULL;
		}

		v16_texec_start_init(me, sge);
	}

	return me;
}

/* Finds the definition of a CPU for a given DI in the v16 conversion context or
 * adds a new entry if the ID is unknown.
 *
 * Returns a pointer to the CPU definition or NULL if allocation fails.
 */
static struct v16_cpu_def*
v16ctx_find_add_cpu_def(struct v16ctx* v16ctx, uint32_t cpu)
{
	struct v16_cpu_def* d;

	if(!(d = v16_cpu_array_bsearch(&v16ctx->cpus, cpu))) {
		if((d = v16_cpu_array_reserve_sorted(&v16ctx->cpus, cpu))) {
			d->cpu = cpu;
			d->numa_node = 0;
			d->numa_node_known = 0;

			if(v16ctx_write_event_collection(v16ctx, cpu))
				return NULL;
		} else {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_ALLOC,
					       "Could not allocate CPU "
					       "definition for CPU "
					       "%" PRIu32 ".", cpu);

		}
	}

	return d;
}

/* Tries to read the a field with with the specified type of the struct s from
 * the input trace. If this operation fails, an error message is pushed onto the
 * error stack of the conversion context and 1 is returned from the function
 * invoking the macro. */
#define READ_FIELD_OR_ERROR_RET1(v16ctx, s, frame_type, field, field_type)	\
	if(am_dsk_##field_type##_read_fp((v16ctx)->fp_in, &(s)->field)) {		\
		am_io_error_stack_push((v16ctx)->estack,			\
				       AM_IOERR_READ_FIELD,			\
				       "Could not field %s "			\
				       "of frame type type %s at offset %jd.",	\
				       #field,					\
				       frame_type,				\
				       ftello((v16ctx)->fp_in));		\
										\
		return 1;							\
	}									\
	do { } while(0)

/* Reads the header of a v16 trace event, skipping the type field. Returns 0 on
 * success, otherwise 1. */
static int
v16ctx_read_convert_event_header(struct v16ctx* v16ctx,
				 struct v16_trace_event_header* h)
{
	READ_FIELD_OR_ERROR_RET1(v16ctx, h, "event header", time, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, h, "event header", cpu, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, h, "event header", worker, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, h, "event header", active_task, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, h, "event header", active_frame, uint64_t);

	v16ctx_check_update_max_timestamp(v16ctx, h->time);

	return 0;
}


/* Reads the header of a v16 trace event, skipping the type field and adds the
 * CPU to the list of known CPUs ov the conversion context if not already
 * present.
 *
 * Returns a pointer to the cpu definition corresponding to the encountered CPU
 * or NULL if allocation fails.
 */
static struct v16_cpu_def*
v16_read_convert_event_header_add_cpu(struct v16ctx* v16ctx,
				      struct v16_trace_event_header* h)
{
	struct v16_cpu_def* cpu_def;

	if(v16ctx_read_convert_event_header(v16ctx, h))
		return NULL;

	if(!(cpu_def = v16ctx_find_add_cpu_def(
		     v16ctx, h->cpu)))
	{
		return NULL;
	}

	return cpu_def;
}

/* Finds the definition of a NUMA node for a given ID in the v16 conversion
 * context or adds a new entry if the ID is unknown.
 *
 * Returns a pointer to the NUMA node definition or NULL if allocation fails.
 */
static struct v16_numa_node_def*
v16ctx_find_add_numa_node(struct v16ctx* v16ctx, uint32_t id)
{
	struct v16_numa_node_def* d;

	if(!(d = v16_numa_node_array_bsearch(&v16ctx->numa_nodes, id))) {
		if((d = v16_numa_node_array_reserve_sorted(&v16ctx->numa_nodes,
							    id)))
		{
			d->id = id;
		} else {
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_ALLOC,
					       "Could not allocate NUMA node "
					       "definition for node "
					       "%" PRIu32 ".", id);
		}
	}

	return d;
}

/* Reads a cpu info event from the input trace and updates the conversion
 * context accordingly (CPU and NUMA node) if necessary.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_cpu_info(struct v16ctx* v16ctx)
{
	struct v16_cpu_def* cpu_def;
	struct v16_trace_cpu_info cpu_info;

	if(!(cpu_def = v16_read_convert_event_header_add_cpu(
		     v16ctx, &cpu_info.header)))
	{
		return 1;
	}

	READ_FIELD_OR_ERROR_RET1(v16ctx, &cpu_info, "CPU info", numa_node, int32_t);

	if(cpu_info.numa_node >= 0) {
		cpu_def->numa_node = cpu_info.numa_node;
		cpu_def->numa_node_known = 1;

		if(!v16ctx_find_add_numa_node(v16ctx, cpu_info.numa_node))
			return 1;
	}

	return 0;
}

/* Reads a frame info event from the input trace and updates the conversion
 * context accordingly (CPU and NUMA node) if necessary. Since the output format
 * does not yet support mappings from addresses to NUMA nodes, the function does
 * not write any data to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_frame_info(struct v16ctx* v16ctx)
{
	/* Not yet supported by the new format; just use CPU */
	struct v16_trace_frame_info fi;

	if(!v16_read_convert_event_header_add_cpu(v16ctx, &fi.header))
		return 1;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &fi, "frame info", addr, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &fi, "frame info", numa_node, int32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &fi, "frame info", size, uint32_t);

	return 0;
}

/* Writes a measurement interval for the specified interval to the output
 * trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_write_measurement_interval(struct v16ctx* v16ctx,
					     const struct am_interval* i)
{
	struct am_dsk_measurement_interval mi;
	uint32_t type = AM_FRAME_TYPE_MEASUREMENT_INTERVAL;

	mi.interval.start = i->start;
	mi.interval.end = i->end;

	if(am_dsk_measurement_interval_write(&v16ctx->octx, type, &mi)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write measurement interval.");
		return 1;
	}

	return 0;
}

/* Reads a global single event from the input trace and writes the corresponding
 * data structure to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_global_single_event(struct v16ctx* v16ctx)
{
	struct v16_trace_global_single_event gsi;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &gsi, "global single event",
				 time, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &gsi, "global single event",
				 single_type, int32_t);

	v16ctx_check_update_max_timestamp(v16ctx, gsi.time);

	switch(gsi.single_type) {
		case V16_GLOBAL_SINGLE_TYPE_MEASURE_START:
			if(v16ctx->measurement_interval_started) {
				am_io_error_stack_push(v16ctx->estack,
						       AM_IOERR_ASSERT,
						       "New measurement "
						       "interval started before "
						       "previous measurement "
						       "interval ended.");
				return 1;
			}

			v16ctx->measurement_interval_started = 1;
			v16ctx->measurement_interval.start = gsi.time;
			break;

		case V16_GLOBAL_SINGLE_TYPE_MEASURE_END:
			if(!v16ctx->measurement_interval_started) {
				am_io_error_stack_push(v16ctx->estack,
						       AM_IOERR_ASSERT,
						       "Measurement end event "
						       "without start event "
						       "found.");
				return 1;
			}

			v16ctx->measurement_interval.end = gsi.time;

			if(v16ctx_write_measurement_interval(
				   v16ctx, &v16ctx->measurement_interval))
			{
				return 1;
			}

			v16ctx->measurement_interval_started = 0;
			break;
		default:
			am_io_error_stack_push(v16ctx->estack,
					       AM_IOERR_READ_FRAME,
					       "Unknown global single event "
					       "type %" PRIu32,
					       gsi.single_type);
			return 1;
	}

	return 0;
}

/* Writes a state description for each default state defined by the v16 format
 * that hasn't been described explicitly with a state description in the input
 * trace.
 *
 * Returns 0 on success, otherwise 1.
 */
int v16ctx_write_missing_default_state_descriptions(struct v16ctx* v16ctx)
{
	struct am_dsk_state_description sd;
	uint64_t tmp;
	size_t idx;
	uint32_t type = AM_FRAME_TYPE_STATE_DESCRIPTION;

	am_for_each_bit_idx_u64(v16ctx->default_states_missing_description,
				tmp,
				idx)
	{
		sd.state_id = idx;
		sd.name.len = strlen(v16_default_state_names[idx]);
		sd.name.str = v16_default_state_names[idx];

		if(am_dsk_state_description_write(&v16ctx->octx, type, &sd))
			return 1;
	}

	return 0;
}

/* Reads a state description (except its type field) from the input trace and
 * writes a corresponding state description to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_state_description(struct v16ctx* v16ctx)
{
	struct v16_trace_state_description sd16;
	struct am_dsk_state_description sd;
	size_t name_size;
	char* name;
	int ret = 1;
	uint32_t type = AM_FRAME_TYPE_STATE_DESCRIPTION;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &sd16, "state description",
				 state_id, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &sd16, "state description",
				 name_len, uint32_t);

	if(am_safe_size_from_u32(&name_size, sd16.name_len)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ASSERT,
				       "State description too long.");
		goto out;
	}

	if(!(name = malloc(name_size))) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Could not allocate space for state "
				       "description");
		goto out;
	}

	if(fread(name, name_size, 1, v16ctx->fp_in) != 1) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Could not read %zu characters of state "
				       "description name at offset %jd",
				       name_size - 1, ftello(v16ctx->fp_in));
		goto out_free;
	}

	sd.state_id = sd16.state_id;
	sd.name.len = sd16.name_len;
	sd.name.str = name;

	if(am_dsk_state_description_write(&v16ctx->octx, type, &sd))
		goto out_free;

	if(sd16.state_id < V16_NUM_DEFAULT_STATES) {
		v16ctx->default_states_missing_description &=
			~(1 << sd16.state_id);
	}

	ret = 0;

out_free:
	free(name);
out:
	return ret;
}

/* Reads a state event (except its type field) from the input trace and writes a
 * corresponding state event to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_state_event(struct v16ctx* v16ctx)
{
	struct v16_trace_state_event se16;
	struct am_dsk_state_event se;
	uint32_t type = AM_FRAME_TYPE_STATE_EVENT;

	if(!v16_read_convert_event_header_add_cpu(v16ctx, &se16.header))
		return 1;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &se16, "state event",
				 end_time, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &se16, "state event", state, uint32_t);

	se.collection_id = se16.header.cpu;
	se.state = se16.state;
	se.interval.start = se16.header.time;
	se.interval.end = se16.end_time;

	v16ctx_check_update_max_timestamp(v16ctx, se16.end_time);

	if(am_dsk_state_event_write(&v16ctx->octx, type, &se)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write state_event.");

		am_io_error_stack_move(v16ctx->estack,
				       &v16ctx->octx.error_stack);

		return 1;
	}

	return 0;
}

/* Reads a counter description (except its type field) from the input trace and
 * writes a corresponding counter description to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_counter_description(struct v16ctx* v16ctx)
{
	struct v16_trace_counter_description cd16;
	struct am_dsk_counter_description cd;
	size_t name_size;
	char* name;
	int ret = 1;
	uint32_t type = AM_FRAME_TYPE_COUNTER_DESCRIPTION;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &cd16, "counter description",
				 counter_id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &cd16, "counter description",
				 name_len, uint32_t);

	if(am_safe_size_from_u32(&name_size, cd16.name_len)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ASSERT,
				       "Counter description too long.");
		goto out;
	}

	if(!(name = malloc(name_size))) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Could not allocate space for counter "
				       "description");
		goto out;
	}

	if(fread(name, name_size, 1, v16ctx->fp_in) != 1) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Could not read %zu characters of counter "
				       "description name at offset %jd",
				       name_size - 1, ftello(v16ctx->fp_in));
		goto out_free;
	}

	if(am_safe_u32_from_u64(&cd.counter_id, cd16.counter_id)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_OVERFLOW,
				       "64-bit counter ID %" PRIu64 " of "
				       "counter description in source trace is "
				       "too big for 32-bit counter ID.",
				       cd16.counter_id);
		goto out_free;
	}

	cd.name.len = cd16.name_len;
	cd.name.str = name;

	if(am_dsk_counter_description_write(&v16ctx->octx, type, &cd)) {
		am_io_error_stack_move(v16ctx->estack,
				       &v16ctx->octx.error_stack);
		goto out_free;
	}

	ret = 0;

out_free:
	free(name);
out:
	return ret;
}

/* Reads a counter event (except its type field) from the input trace and writes
 * a corresponding counter event to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_counter_event(struct v16ctx* v16ctx)
{
	struct v16_trace_counter_event ce16;
	struct am_dsk_counter_event ce;
	uint32_t type = AM_FRAME_TYPE_COUNTER_EVENT;

	if(!v16_read_convert_event_header_add_cpu(v16ctx, &ce16.header))
		return 1;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "counter event", counter_id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "counter event", value, int64_t);
	ce.collection_id = ce16.header.cpu;
	ce.counter_id = ce16.counter_id;
	ce.time = ce16.header.time;
	ce.value = ce16.value;

	if(am_dsk_counter_event_write(&v16ctx->octx, type, &ce)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write counter_event.");

		am_io_error_stack_move(v16ctx->estack, &v16ctx->octx.error_stack);

		return 1;
	}

	return 0;
}

/* Reads an OpenMP for instance event (except its type field) from the
 * input. Since OpenMP is not yet supported by the current trace format, nothing
 * is written to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_omp_for_instance(struct v16ctx* v16ctx)
{
	/* Currently not supported; just skip */
	struct v16_trace_omp_for_instance ofi16;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 flags, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 addr, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 increment, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 lower_bound, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 upper_bound, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofi16, "OMP for instance",
				 num_workers, uint32_t);

	return 0;
}

/* Reads an OpenMP for chunk set (except its type field) from the input. Since
 * OpenMP is not yet supported by the current trace format, nothing is written
 * to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_omp_for_chunk_set(struct v16ctx* v16ctx)
{
	/* Currently not supported; just skip */
	struct v16_trace_omp_for_chunk_set ofcs16;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcs16, "OMP for chunk set",
				 for_id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcs16, "OMP for chunk set",
				 id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcs16, "OMP for chunk set",
				 first_lower, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcs16, "OMP for chunk set",
				 first_upper, uint64_t);

	return 0;
}

/* Reads an OpenMP for chunk set part (except its type field) from the
 * input. Since OpenMP is not yet supported by the current trace format, nothing
 * is written to the output trace, only the CPU of the input event is
 * registered.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_omp_for_chunk_set_part(struct v16ctx* v16ctx)
{
	/* Currently not supported; just skip */
	struct v16_trace_omp_for_chunk_set_part ofcsp16;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcsp16, "OMP for chunk set part",
				 cpu, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcsp16, "OMP for chunk set part",
				 chunk_set_id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcsp16, "OMP for chunk set part",
				 start, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ofcsp16, "OMP for chunk set part",
				 end, uint64_t);

	v16ctx_check_update_max_timestamp(v16ctx, ofcsp16.end);

	if(!v16ctx_find_add_cpu_def(v16ctx, ofcsp16.cpu))
		return 1;

	return 0;
}

/* Adds an entry to the array of work function addresses for which an OpenMP
 * task type frame has already been written. Returns 0 on success, otherwise
 * 1. */
static inline int
v16_omp_task_type_address_add(struct v16ctx* v16ctx, uint64_t addr)
{
	uint64_t* pval;

	if(!(pval = u64_array_reserve_sorted(&v16ctx->omp_task_type_addresses,
					     addr)))
	{
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Could not allocate task type address "
				       "array entry for OpenMP task with "
				       "address 0x%" PRIx64 ".",
				       addr);

		return 1;
	}

	*pval = addr;

	return 0;
}

/* Writes an OpenMP task type frame if such a frame hasn't already been written
 * previusly for the work function with the given address. Returns 0 on success,
 * otherwise 1. */
static inline int v16_write_omp_task_type_if_necessary(struct v16ctx* v16ctx,
						       uint64_t addr)
{
	struct am_dsk_openmp_task_type tt;
	char buf[64];
	uint32_t type = AM_FRAME_TYPE_OPENMP_TASK_TYPE;

	if(u64_array_bsearch(&v16ctx->omp_task_type_addresses, addr))
		return 0;

	snprintf(buf, sizeof(buf), "omp_task_0x%" PRIx64, addr);

	/* Write task type. Use address of the work function as type ID */
	tt.type_id = addr;
	tt.name.str = buf;
	tt.name.len = strlen(buf);
	tt.source.file.str = "";
	tt.source.file.len = 0;
	tt.source.line = 0;
	tt.source.character = 0;

	if(am_dsk_openmp_task_type_write(&v16ctx->octx, type, &tt)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write OpenMP task type.");
		return 1;
	}

	if(v16_omp_task_type_address_add(v16ctx, addr))
		return 1;

	return 0;
}

/* Reads an OpenMP task instance (except its type field) from the input trace
 * and writes a corresponding OpenMP task instance to the output trace. If no
 * frame for the task type has been written to the output trace before, a frame
 * for the task type is also written to the trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_omp_task_instance(struct v16ctx* v16ctx)
{
	struct v16_trace_omp_task_instance oti16;
	struct am_dsk_openmp_task_instance ti;
	uint32_t ti_type = AM_FRAME_TYPE_OPENMP_TASK_INSTANCE;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &oti16, "OMP task instance",
				 addr, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &oti16, "OMP task instance",
				 id, uint64_t);

	if(v16_write_omp_task_type_if_necessary(v16ctx, oti16.addr))
		return 1;

	ti.type_id = oti16.addr;
	ti.instance_id = oti16.id;

	if(am_dsk_openmp_task_instance_write(&v16ctx->octx, ti_type, &ti)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write OpenMP task "
				       "instance.");
		return 1;
	}

	return 0;
}

/* Reads an OpenMP task instance part (except its type field) from the input and
 * writes a corresponding OpenMP task execution period to the output trace.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_omp_task_instance_part(struct v16ctx* v16ctx)
{
	struct v16_trace_omp_task_instance_part otip16;
	struct am_dsk_openmp_task_period tp;
	uint32_t tp_type = AM_FRAME_TYPE_OPENMP_TASK_PERIOD;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &otip16, "OMP task instance part",
				 cpu, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &otip16, "OMP task instance part",
				 task_instance_id, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &otip16, "OMP task instance part",
				 start, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &otip16, "OMP task instance part",
				 end, uint64_t);

	v16ctx_check_update_max_timestamp(v16ctx, otip16.end);

	if(!v16ctx_find_add_cpu_def(v16ctx, otip16.cpu))
		return 1;

	tp.collection_id = otip16.cpu;
	tp.instance_id = otip16.task_instance_id;
	tp.interval.start = otip16.start;
	tp.interval.end = otip16.end;

	if(am_dsk_openmp_task_period_write(&v16ctx->octx, tp_type, &tp)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write OpenMP task period "
				       "for CPU %" PRIu32 " with interval "
				       "[%" PRIu64 ", %" PRIu64 "].",
				       otip16.cpu,
				       otip16.start,
				       otip16.end);
		return 1;
	}

	return 0;
}

/* Reads a comm event (except its type field) from the input. Since
 * communication events are not yet supported by the current trace format,
 * nothing is written to the output trace, only the source and destination CPUs
 * of the input event are registered.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_comm_event(struct v16ctx* v16ctx)
{
	/* Currently not supported; just skip */
	struct v16_trace_comm_event ce16;

	if(v16ctx_read_convert_event_header(v16ctx, &ce16.header))
		return 1;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "comm event", type, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "comm event",
				 src_or_dst_cpu, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "comm event", size, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "comm event", prod_ts, uint64_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &ce16, "comm event", what, uint64_t);

	if(!v16ctx_find_add_cpu_def(v16ctx, ce16.src_or_dst_cpu))
		return 1;

	v16ctx_check_update_max_timestamp(v16ctx, ce16.prod_ts);

	return 0;
}

/* Processes a single event defining a task execution start event. The frames
 * for the task type, task instance and task period are written when the
 * corresponding single event for the end of the execution is processed.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_texec_start(struct v16ctx* v16ctx,
				      struct v16_trace_single_event* sge16)
{
	struct v16_texec_start* me;

	if(!(me = v16_texec_start_array_find_add(
		     &v16ctx->per_cpu.last_texec_start, v16ctx, sge16)))
	{
		return 1;
	}

	if(me->valid) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Found two consecutive texec start "
				       "events without texec end event in "
				       "between for CPU %" PRIu32 ".",
				       sge16->header.cpu);
		return 1;
	}

	v16_texec_start_init(me, sge16);
	me->valid = 1;

	return 0;
}

/* Processes a single event defining a task execution end event and writes the
 * frames for the task type (if necessary), task instance and task period.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_texec_end(struct v16ctx* v16ctx,
				    struct v16_trace_single_event* sge16_end)
{
	struct v16_texec_start* start;
	uint32_t cpu = sge16_end->header.cpu;
	uint64_t workfn_addr = sge16_end->what;
	struct am_dsk_openstream_task_instance ti;
	struct am_dsk_openstream_task_period tp;
	uint32_t ti_type = AM_FRAME_TYPE_OPENSTREAM_TASK_INSTANCE;
	uint32_t tp_type = AM_FRAME_TYPE_OPENSTREAM_TASK_PERIOD;

	start = v16_texec_start_array_bsearch(&v16ctx->per_cpu.last_texec_start,
					       cpu);

	if(!start || !start->valid) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Found texec end event without texec "
				       "start event for CPU %" PRIu32 ".",
				       cpu);
		return 1;
	}

	if(sge16_end->header.time <= start->time) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Found texec end event with timestamp "
				       "smaller than or equal to the timestamp "
				       "of the corresponding texec start event "
				       "for CPU %" PRIu32 ".",
				       cpu);
		return 1;
	}

	if(workfn_addr != start->workfn) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ALLOC,
				       "Found texec end event for a different "
				       "task than the preceding texec start "
				       "event for CPU %" PRIu32 ".", cpu);
		return 1;
	}

	if(v16_write_openstream_task_type_if_necessary(v16ctx, workfn_addr))
		return 1;

	/* Create OpenStream task instance. The instance will contain only a
	 * single task execution period, since v16 did not allow for capturing
	 * task suspension. */
	ti.type_id = workfn_addr;
	ti.instance_id = ++(v16ctx->curr_id);

	if(am_dsk_openstream_task_instance_write(&v16ctx->octx, ti_type, &ti)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write OpenStream task "
				       "instance.");
		return 1;
	}

	/* Create OpenStream task period. */
	tp.collection_id = cpu;
	tp.instance_id = ti.instance_id;
	tp.interval.start = start->time;
	tp.interval.end = sge16_end->header.time;

	if(am_dsk_openstream_task_period_write(&v16ctx->octx, tp_type, &tp)) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_WRITE,
				       "Could not write OpenStream task period "
				       "for CPU %" PRIu32 " with interval "
				       "[%" PRIu64 ", %" PRIu64 "].",
				       cpu,
				       start->time,
				       sge16_end->header.time);
		return 1;
	}

	start->valid = 0;

	return 0;
}

/* Reads a single event (except its type field) from the input. Since the
 * corresponding frame types not yet defined for the current trace format,
 * nothing is written to the output trace, only the CPU of the input event is
 * registered.
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_convert_single_event(struct v16ctx* v16ctx)
{
	struct v16_trace_single_event sge16;

	if(v16ctx_read_convert_event_header(v16ctx, &sge16.header))
		return 1;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &sge16, "single event", type, uint32_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &sge16, "single event", what, uint64_t);

	switch(sge16.type) {
		case V16_SINGLE_TYPE_TEXEC_START:
			return v16ctx_convert_texec_start(v16ctx, &sge16);
		case V16_SINGLE_TYPE_TEXEC_END:
			return v16ctx_convert_texec_end(v16ctx, &sge16);

		/* Not yet supported; just skip */
		case V16_SINGLE_TYPE_TDESTROY:
		case V16_SINGLE_TYPE_TCREATE:
			break;
	}

	return 0;
}

/* Converts all directly convertible samples of the input file and writes the
 * corresponding frames to the output file.
 *
 * Returns 0 on success, otherwise 1.
 */
int v16ctx_convert_samples(struct v16ctx* v16ctx)
{
	uint32_t event_type;

	while(!feof(v16ctx->fp_in)) {
		if(am_dsk_uint32_t_read_fp(v16ctx->fp_in, &event_type)) {
			if(feof(v16ctx->fp_in)) {
				/* End reached */
				return 0;
			} else {
				am_io_error_stack_push(v16ctx->estack,
						       AM_IOERR_READ,
						       "Could not read event "
						       "type.");
				return 1;
			}
		}

		switch(event_type) {
			case V16_EVENT_TYPE_STATE:
				if(v16ctx_convert_state_event(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_COMM:
				if(v16ctx_convert_comm_event(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_SINGLE:
				if(v16ctx_convert_single_event(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_COUNTER:
				if(v16ctx_convert_counter_event(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_COUNTER_DESCRIPTION:
				if(v16ctx_convert_counter_description(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_FRAME_INFO:
				if(v16ctx_convert_frame_info(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_CPU_INFO:
				if(v16ctx_convert_cpu_info(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_GLOBAL_SINGLE_EVENT:
				if(v16ctx_convert_global_single_event(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_STATE_DESCRIPTION:
				if(v16ctx_convert_state_description(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_OMP_FOR:
				if(v16ctx_convert_omp_for_instance(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_OMP_FOR_CHUNK_SET:
				if(v16ctx_convert_omp_for_chunk_set(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_OMP_FOR_CHUNK_SET_PART:
				if(v16ctx_convert_omp_for_chunk_set_part(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_OMP_TASK_INSTANCE:
				if(v16ctx_convert_omp_task_instance(v16ctx))
					return 1;
				break;
			case V16_EVENT_TYPE_OMP_TASK_INSTANCE_PART:
				if(v16ctx_convert_omp_task_instance_part(v16ctx))
					return 1;
				break;
			default:
				am_io_error_stack_push(v16ctx->estack,
						       AM_IOERR_LOAD_FRAME,
						       "Unknown event type "
						       "%" PRIu32 ".",
						       event_type);
				return 1;
		}
	}

	return 1;
}

/* Discards the rest of the input file header (i.e., all fields, except the
 * magic number and the trace version).
 *
 * Returns 0 on success, otherwise 1.
 */
static int v16ctx_discard_input_file_header(struct v16ctx* v16ctx)
{
	struct v16_trace_header h;

	READ_FIELD_OR_ERROR_RET1(v16ctx, &h, "trace header", day, uint8_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &h, "trace header", month, uint8_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &h, "trace header", year, uint16_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &h, "trace header", hour, uint8_t);
	READ_FIELD_OR_ERROR_RET1(v16ctx, &h, "trace header", minute, uint8_t);

	return 0;
}

/* Checks if the conversion context is in a valid state at the end of the
 * conversion.
 *
 * Returns 0 on success, otherwise 1.
 */
int v16ctx_final_check(struct v16ctx* v16ctx)
{
	if(v16ctx->measurement_interval_started) {
		am_io_error_stack_push(v16ctx->estack,
				       AM_IOERR_ASSERT,
				       "Missing matching end event for "
				       "measurement interval.");

		return 1;
	}

	return 0;
}

/* Converts a trace in format version 13, 14, 15 or 16 to the current format.
 * fp_in must be positioned after the file version field of the file header
 * and fp_out must be positioned at the beginning of the output file. Errors
 * are reported using the I/O error stack estack.
 *
 * Returns 0 on success, otherwise 1.
 */
int v16_convert_trace(FILE* fp_in, FILE* fp_out, struct am_io_error_stack* es)
{
	struct v16ctx v16ctx;
	int ret = 1;

	if(v16ctx_init(&v16ctx, fp_in, fp_out, es))
		goto out;

	if(v16ctx_discard_input_file_header(&v16ctx))
		goto out_destroy;

	if(v16ctx_write_file_header(&v16ctx))
		goto out_destroy;

	if(v16ctx_register_frame_types(&v16ctx))
		goto out_destroy;

	if(v16ctx_convert_samples(&v16ctx))
		goto out_destroy;

	if(v16ctx_write_missing_default_state_descriptions(&v16ctx))
		goto out_destroy;

	if(v16ctx_write_hierarchy(&v16ctx))
		goto out_destroy;

	if(v16ctx_write_event_mappings(&v16ctx))
		goto out_destroy;

	if(v16ctx_final_check(&v16ctx))
		goto out_destroy;

	ret = 0;

out_destroy:
	if(ret != 0)
		am_io_error_stack_move(es, &v16ctx.octx.error_stack);

	v16ctx_destroy(&v16ctx);
out:
	return ret;
}
