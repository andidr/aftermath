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

#ifndef AM_DFG_NODE_H
#define AM_DFG_NODE_H

#include <stdarg.h>
#include <aftermath/core/dfg_type.h>
#include <aftermath/core/dfg_type_registry.h>
#include <aftermath/core/dfg_buffer.h>
#include <aftermath/core/object_notation.h>
#include <aftermath/core/typed_rbtree.h>

struct am_dfg_port_mask {
	/* For port masks embedded into nodes:
	 *
	 *   Included input ports have been marked to consume data from the
	 *   connected producer only of the data has changed since the last time
	 *   the connected output port pushed data. If the connected output port
	 *   does not indicate that it will produce new data, data generation at
	 *   the output port can be omitted.
	 *
	 * For port masks embedded into port types:
	 *
	 *   When the port is requested, mark included input ports to pull data
	 *   in if the data on the connected output port has changed since the
	 *   last push of that output port.
	 */
	uint64_t pull_new;

	/* For port masks embedded into nodes:
	 *
	 *   Included input ports have been marked to pull data in from the
	 *   connected output port, even if this data hasn't changed since the
	 *   last time the input port was pulled.
	 *
	 * For port masks embedded into port types:
	 *
	 *   When the port is requested, mark included input ports to pull data
	 *   in, even if the connected output port would provide the same data
	 *   as the data from its last push.
	 */
	uint64_t pull_old;

	/* For port masks embedded into nodes:
	 *
	 *   Included output ports will or are likely to produce new data. This
	 *   might be conservative choice and the node might actually provide
	 *   the same data as the last time at the indicated ports.
	 *
	 * For port masks embedded into port types:
	 *
	 *   When the port is requested, mark included output ports as ports
	 *   that might push new data.
	 */
	uint64_t push_new;

	/* For port masks embedded into nodes:
	 *
	 *   Included output ports will produce the same data as at the last
	 *   push.
	 *
	 * For port masks embedded into port types:
	 *
	 *   When the port is requested, mark included output ports as ports
	 *   that will provide the same data as at the last push.
	 */
	uint64_t push_old;
};

/* Sets all fields fo a port masl k to 0 */
static inline void am_dfg_port_mask_reset(struct am_dfg_port_mask* m)
{
	memset(m, 0, sizeof(*m));
}

/* Copies all bitmaps of a port mask src to dst */
static inline void am_dfg_port_mask_copy(struct am_dfg_port_mask* dst,
					 const struct am_dfg_port_mask* src)
{
	*dst = *src;
}

/* Copies all values of src to dst using bitwise or */
static inline void am_dfg_port_mask_apply(struct am_dfg_port_mask* dst,
					  const struct am_dfg_port_mask* src)
{
	dst->pull_new |= src->pull_new;
	dst->pull_old |= src->pull_old;
	dst->push_new |= src->push_new;
	dst->push_old |= src->push_old;
}

/* Sets the bits in the bitmaps of dst to 1 where the corresponding bitmaps of a
 * and b have different values */
static inline void am_dfg_port_mask_diff(struct am_dfg_port_mask* dst,
					 const struct am_dfg_port_mask* a,
					 const struct am_dfg_port_mask* b)
{
	dst->pull_new = a->pull_new ^ b->pull_new;
	dst->pull_old = a->pull_old ^ b->pull_old;
	dst->push_new = a->push_new ^ b->push_new;
	dst->push_old = a->push_old ^ b->push_old;
}

/* Returns true if all bits of all bitmaps of sub set to 1 also have the value 1
 * in sup. Otherwise, the function returns 0. */
static inline int am_dfg_port_mask_is_subset(const struct am_dfg_port_mask* sup,
					     const struct am_dfg_port_mask* sub)
{
	return ((sub->pull_new & sup->pull_new) == sub->pull_new &&
		(sub->pull_old & sup->pull_old) == sub->pull_old &&
		(sub->push_new & sup->push_new) == sub->push_new &&
		(sub->push_old & sup->push_old) == sub->push_old);
}

enum am_dfg_port_flag {
	AM_DFG_PORT_IN = (1 << 0),
	AM_DFG_PORT_OUT = (1 << 1),
	AM_DFG_PORT_MANDATORY = (1 << 2)
};

struct am_dfg_port_type {
	/* Name of the port type */
	char* name;

	/* Type of the data transported on the port */
	const struct am_dfg_type* type;
	long flags;
};

/* Returns true if a port type pt defines an input port. */
static inline int
am_dfg_port_type_is_input_type(const struct am_dfg_port_type* pt)
{
	return pt->flags & AM_DFG_PORT_IN;
}

/* Returns true if a port type pt defines an output port. */
static inline int
am_dfg_port_type_is_output_type(const struct am_dfg_port_type* pt)
{
	return pt->flags & AM_DFG_PORT_OUT;
}

/* Static definition of a DFG node port type */
struct am_dfg_static_port_type_def {
	/* Name of the port */
	const char* name;

	/* Name of the type of the port's elements */
	const char* type_name;

	long flags;
};

int am_dfg_port_type_init(struct am_dfg_port_type* pt,
			  const char* name,
			  const struct am_dfg_type* type,
			  long flags);

void am_dfg_port_type_destroy(struct am_dfg_port_type* pt);

/* An actual instance of a port */
struct am_dfg_port {
	/* The node that this port belongs to */
	struct am_dfg_node* node;

	/* The type of this port */
	const struct am_dfg_port_type* type;

	/* The data present at the port */
	struct am_dfg_buffer* buffer;

	/* Incoming / outgoing connections. If the port is an in port, there is
	 * at most one incoming connection. */
	struct am_dfg_port** connections;

	/* The number of connections */
	size_t num_connections;
};

/* Iterates over all ports of n included in the bitmask m. The variable p is
 * used as the iterator and tmp must be a temporary uint64_t*/
#define am_dfg_node_for_each_masked_port(n, m, p, tmp)				\
	for(tmp = (m), p = &(n)->ports[am_first_set_bit_idx_u64(tmp)];		\
	    p != &(n)->ports[64];						\
	    tmp = tmp & (~(UINT64_C(1) << am_first_set_bit_idx_u64(tmp))),	\
		    p = &(n)->ports[am_first_set_bit_idx_u64(tmp)])

/* A directed connection between two ports. */
struct am_dfg_connection {
	/* Source port */
	struct am_dfg_port* src;

	/* Destination port */
	struct am_dfg_port* dst;
};

/* Returns true if two connections a and b are identical */
static inline int am_dfg_connection_eq(const struct am_dfg_connection* a,
				       const struct am_dfg_connection* b)
{
	return (a->src == b->src && a->dst == b->dst);
}

/* Returns true if two connections a and b connect the same ports, ignoring
 * their direction */
static inline int am_dfg_connection_eq_nd(const struct am_dfg_connection* a,
					  const struct am_dfg_connection* b)
{
	return (a->src == b->src && a->dst == b->dst) ||
		(a->src == b->dst && a->dst == b->src);
}

#define am_dfg_port_for_each_connected_port_safe(p, i, c)	\
	for((c) = 0, (i) = ((p)->num_connections > 0) ?	\
		    (p)->connections[0] :			\
		    NULL;					\
	    (c) < (p)->num_connections;			\
	    (c)++, i = ((p)->num_connections > (c)) ?		\
		    (p)->connections[(c)] :			\
		    NULL)

int am_dfg_port_connect_onesided(struct am_dfg_port* p, struct am_dfg_port* other);
int am_dfg_port_disconnect(struct am_dfg_port* p_out, struct am_dfg_port* p_in);
int am_dfg_port_disconnect_onesided(struct am_dfg_port* p, struct am_dfg_port* other);
void am_dfg_port_destroy(struct am_dfg_port* p);

/* Returns true if a port p has at least one connection. */
static inline int am_dfg_port_is_connected(const struct am_dfg_port* p)
{
	return p->num_connections > 0;
}

/* Returns true if a port p is connected to another port pother. */
static inline int am_dfg_ports_connected(const struct am_dfg_port* p,
					 const struct am_dfg_port* pother)
{
	const struct am_dfg_port* piter;
	size_t i;

	am_dfg_port_for_each_connected_port_safe(p, piter, i)
		if(piter == pother)
			return 1;

	return 0;
}

/* Returns true if a port p is an input port. */
static inline int am_dfg_port_is_input_port(const struct am_dfg_port* p)
{
	return am_dfg_port_type_is_input_type(p->type);
}

/* Returns true if a port p is an output port. */
static inline int am_dfg_port_is_output_port(const struct am_dfg_port* p)
{
	return am_dfg_port_type_is_output_type(p->type);
}

/* If a is an input port and b is an output port or the other way around,
 * *in_port and *out_port will be set to the in and out port, respectively and
 * the function returns 0. If a and b are both input ports or both ouput ports,
 * the function does nothing and returns 1. */
static inline int am_dfg_sort_ports_inout(const struct am_dfg_port* a,
					  const struct am_dfg_port* b,
					  struct am_dfg_port const ** out_port,
					  struct am_dfg_port const ** in_port)
{
	if(am_dfg_port_is_output_port(a) &&
	   am_dfg_port_is_input_port(b))
	{
		*in_port = b;
		*out_port = a;

		return 0;
	} else if(am_dfg_port_is_input_port(a) &&
		  am_dfg_port_is_output_port(b))
	{
		*in_port = a;
		*out_port = b;

		return 0;
	}

	return 1;
}

/* Static definition of a property */
struct am_dfg_static_property_def {
	/* Name of the property */
	const char* name;

	/* Human-readable name of the property */
	const char* hrname;

	/* Name of the type of the property */
	const char* type_name;
};

/* Property of a dfg node */
struct am_dfg_property {
	/* Name of the property */
	char* name;

	/* Human-readable name of the property */
	char* hrname;

	/* The type of the property */
	const struct am_dfg_type* type;
};

int am_dfg_property_init(struct am_dfg_property* p,
			 const char* name,
			 const char* hrname,
			 const struct am_dfg_type* type);

void am_dfg_property_destroy(struct am_dfg_property* p);

struct am_dfg_node;

struct am_dfg_node_type_functions {
	/* Function called when a node of this type is allocated */
	struct am_dfg_node* (*allocate)(void);

	/* Function called when a node of this type is initialized */
	int (*init)(struct am_dfg_node* n);

	/* Function called when a node of this type is destroyed */
	void (*destroy)(struct am_dfg_node* n);

	/* Function called when a node of this type is scheduled for
	 * execution */
	int (*process)(struct am_dfg_node* n);

	/* Function called when a port of a node of this type is connected */
	void (*connect)(struct am_dfg_node* n, struct am_dfg_port* pi);

	/* Function called when a port of a node of this type is disconnected */
	void (*disconnect)(struct am_dfg_node* n, struct am_dfg_port* pi);

	/* Function called when a node of this type is constructed from an
	 * object notation group node. The function is called after the init
	 * function. A return value of 0 indicates a success, a return value of
	 * 1 indicates an error. */
	int (*from_object_notation)(struct am_dfg_node* n,
				    struct am_object_notation_node_group* g);

	/* Function called when a node of this type is needs to be converted to
	 * an object notation group node. Upon a call, g is already a valid
	 * group node with the basic members initialized. A return value of 0
	 * indicates a success, a return value of 1 indicates an error. */
	int (*to_object_notation)(struct am_dfg_node* n,
				  struct am_object_notation_node_group* g);

	/* Function called when a property is set externally (e.g., from a
	 * controller reading a property from a dialog box). A return value of 0
	 * indicates that the property has been set correctly. On failure, a
	 * value different from 0 should be returned. */
	int (*set_property)(struct am_dfg_node* n,
			    const struct am_dfg_property* property,
			    const void* value);

	/* Function called when the value of a property requested from the
	 * node. The node remains the owner of the value and must only provide a
	 * read-only pointer to it. */
	int (*get_property)(const struct am_dfg_node* n,
			    const struct am_dfg_property* property,
			    void** value);
};

/* A node type */
struct am_dfg_node_type {
	/* The name of this node type */
	char* name;

	/* The human-redable name of this type */
	char* hrname;

	/* The types of the ports associated to this node type */
	struct am_dfg_port_type* ports;

	/* The number of ports */
	size_t num_ports;

	long flags;

	/* Functions associated to the node type (processing, allocations, port
	 * connections, etc.) */
	struct am_dfg_node_type_functions functions;

	/* Chaining of all node types */
	struct list_head list;

	/* Size in bytes of an instance of this node type */
	size_t instance_size;

	/* Number of properties of a node of this type */
	size_t num_properties;

	/* List of property names */
	struct am_dfg_property* properties;
};

/* Default size of a node instance */
#define AM_DFG_NODE_DEFAULT_SIZE sizeof(struct am_dfg_node)

/* Static definition of a DFG node type */
struct am_dfg_static_node_type_def {
	/* Name of the declared node type */
	const char* name;

	/* Human-readable name of the declared node type */
	const char* hrname;

	/* Size of an instance of this node in bytes */
	size_t instance_size;

	/* The types' functions */
	struct am_dfg_node_type_functions functions;

	/* Number of ports */
	size_t num_ports;

	/* Pointer to a static array of port definitions for the node type */
	struct am_dfg_static_port_type_def* ports;

	/* Number of properties */
	size_t num_properties;

	/* Pointer to a static array of property definitions for the node
	 * type */
	struct am_dfg_static_property_def* properties;
};

#define am_dfg_node_for_each_port(n, p)		\
	for((p) = &(n)->ports[0];			\
	    (p) != &(n)->ports[(n)->type->num_ports];	\
	    (p)++)

#define am_dfg_node_for_each_property(n, p)				\
	for((p) = &(n)->type->properties[0];				\
	    (p) != &(n)->type->properties[(n)->type->num_properties];	\
	    (p)++)

void am_dfg_node_type_destroy(struct am_dfg_node_type* nt);

int am_dfg_node_type_builds(struct am_dfg_node_type* nt,
			    struct am_dfg_type_registry* reg,
			    const struct am_dfg_static_node_type_def* sdef);

int am_dfg_node_type_buildv(struct am_dfg_node_type* nt,
			    struct am_dfg_type_registry* reg,
			    const char* name,
			    const char* hrname,
			    size_t instance_size,
			    size_t num_ports,
			    size_t num_properties,
			    va_list arg);

int am_dfg_node_type_build(struct am_dfg_node_type* nt,
			   struct am_dfg_type_registry* reg,
			   const char* name,
			   const char* hrname,
			   size_t instance_size,
			   size_t num_ports,
			   size_t num_properties,
			   ...);

struct am_dfg_port_type*
am_dfg_node_type_find_port_type(const struct am_dfg_node_type* nt,
				const char* port_name);

/* Iterates over all port types of a node type */
#define am_dfg_node_type_for_each_port_type(nt, pt)	\
	for((pt) = &(nt)->ports[0];			\
	    (pt) != &(nt)->ports[(nt)->num_ports];	\
	    (pt)++)

/* Returns a bit mask with bits set to 1 at indexes that correspond to the
 * indexes of input ports in the node type's array of ports
 *
 * FIXME: Use bit masks to indicate input / output ports anyways, such that this
 * becomes a simple function returning the input mask.
 */
static inline uint64_t
am_dfg_node_type_input_mask(const struct am_dfg_node_type* nt)
{
	struct am_dfg_port_type* pt;
	uint64_t mask = 0;
	size_t idx = 0;

	am_dfg_node_type_for_each_port_type(nt, pt) {
		if(am_dfg_port_type_is_input_type(pt))
			mask |= UINT64_C(1) << idx;

		idx++;
	}

	return mask;
}

/* Returns a bit mask with bits set to 1 at indexes that correspond to the
 * indexes of output ports in the node type's array of ports
 *
 * FIXME: Use bit masks to indicate input / output ports anyways, such that this
 * becomes a simple function returning the output mask.
 */
static inline uint64_t
am_dfg_node_type_output_mask(const struct am_dfg_node_type* nt)
{
	struct am_dfg_port_type* pt;
	uint64_t mask = 0;
	size_t idx = 0;

	am_dfg_node_type_for_each_port_type(nt, pt) {
		if(am_dfg_port_type_is_output_type(pt))
			mask |= UINT64_C(1) << idx;

		idx++;
	}

	return mask;
}

/* Returns the zero-based index of the port type pt within the node type nt. */
static inline size_t am_dfg_port_type_index(const struct am_dfg_port_type* pt,
					    const struct am_dfg_node_type* nt)
{
	return AM_ARRAY_INDEX(nt->ports, pt);
}

/* Returns a mask with the bit set to 1 at the position correspondign to the
 * index of the port type pt within the node type nt. */
static inline uint64_t
am_dfg_port_type_mask_bits(const struct am_dfg_port_type* pt,
			   const struct am_dfg_node_type* nt)
{
	return UINT64_C(1) << am_dfg_port_type_index(pt, nt);
}

/* Instance of a node */
struct am_dfg_node {
	/* The node's node type */
	const struct am_dfg_node_type* type;

	/* Tree sorted by node IDs */
	struct rb_node rb_node;

	/* The node's port instances */
	struct am_dfg_port* ports;

	/* Number of remaining dependencies (used for scheduling) */
	size_t num_deps_remaining;

	/* Marking used by the scheduler / cycle checker */
	int marking;

	/* The id of this node instance */
	long id;
};

/* Returns the zero-based index of the port p within its node. */
static inline size_t am_dfg_port_index(const struct am_dfg_port* p)
{
	return AM_ARRAY_INDEX(p->node->ports, p);
}

/* Returns a mask with the bit set to one that corresponds to the index of p
 * within its node. */
static inline uint64_t am_dfg_port_mask_bits(const struct am_dfg_port* p)
{
	return UINT64_C(1) << am_dfg_port_index(p);
}


/* Returns true if at least one sample is present at port p. Otherwise, returns
 * false. */
static inline int am_dfg_port_has_data(const struct am_dfg_port* p)
{
	return p->buffer->num_samples > 0;
}

/* Given a node n and one of its ports curr, the function finds the next port
 * whose flags include all the flags specified in a mask. If no such port is
 * exists, the function returns NULL.
 */
static inline struct am_dfg_port*
am_dfg_node_next_port_flagged(const struct am_dfg_node* n,
			      long mask,
			      struct am_dfg_port* curr)
{
	if(n->type->num_ports == 0)
		return NULL;

	curr = (!curr) ? &n->ports[0] : curr+1;

	do {
		/* Check that we haven't gone past the last port */
		if(((uintptr_t)curr) >
		   ((uintptr_t)&n->ports[n->type->num_ports-1]))
			return NULL;

		if(curr->type->flags & mask)
			return curr;

		curr++;
	} while(1);

	return NULL;
}

/* Iterates over all ports of a node n whose flags include the flags specified
 * in the mask. */
#define am_dfg_node_for_each_port_flagged(n, p, mask)		\
	for((p) = am_dfg_node_next_port_flagged(n, mask, NULL); \
	    (p);						\
	    (p) = am_dfg_node_next_port_flagged(n, mask, (p)))

/* Iterates over all input ports of a node n */
#define am_dfg_node_for_each_input_port(n, p) \
	am_dfg_node_for_each_port_flagged(n, p, AM_DFG_PORT_IN)

/* Iterates over all output ports of a node n */
#define am_dfg_node_for_each_output_port(n, p) \
	am_dfg_node_for_each_port_flagged(n, p, AM_DFG_PORT_OUT)

void am_dfg_node_reset_sched_data(struct am_dfg_node* n);
struct am_dfg_node* am_dfg_node_alloc(const struct am_dfg_node_type* t);
int am_dfg_node_instantiate(struct am_dfg_node* n,
			    const struct am_dfg_node_type* t,
			    long id);
size_t am_dfg_node_get_port_index(const struct am_dfg_node* n,
				  const struct am_dfg_port* p);
void am_dfg_node_destroy(struct am_dfg_node* n);
struct am_dfg_port* am_dfg_node_find_port(const struct am_dfg_node* n,
					  const char* name);
int am_dfg_node_is_well_connected(const struct am_dfg_node* n);
int am_dfg_node_is_root(const struct am_dfg_node* n);
int am_dfg_node_is_root_ign(const struct am_dfg_node* n,
			    const struct am_dfg_port* ignore_src,
			    const struct am_dfg_port* ignore_dst);

struct am_object_notation_node*
am_dfg_node_to_object_notation(struct am_dfg_node* n);

struct am_dfg_node*
am_dfg_node_from_object_notation(struct am_dfg_node_type* nt,
				 struct am_object_notation_node_group* g);

/* When we're not using the definitions, i.e., in all code outside of the
 * translation unit defining the builtin node types, expand node type
 * declarations to no-ops. */
#ifndef AM_DFG_GEN_BUILTIN_NODES
	#define AM_DFG_DECL_BUILTIN_NODE_TYPE_SWITCH(...)
	#define AM_DFG_ADD_BUILTIN_NODE_TYPES_SWITCH(...)
#endif

#define AM_DFG_NODE_PORTS(...) (__VA_ARGS__)
#define AM_DFG_NODE_PROPERTIES(...) (__VA_ARGS__)
#define AM_DFG_NODE_FUNCTIONS(...) (__VA_ARGS__)

/* Generates a static definition for a DFG node type. ID must be a globally
 * unique identifier for the node type. This identifier is only used during
 * compilation and does not appear as a DFG node type. NODE_TYPE_NAME must be
 * set to the name of the node that is declared. This name will appear in the
 * DFG node namespace of a node type registry. NODE_TYPE_HRNAME is the
 * human-redable name for the node type. INSTANCE_SIZE is the size in bytes of
 * an instance of this node type. FUNCTIONS must be a static initializer for a
 * struct am_dfg_node_type_functions. PORTS is a list of port definitions
 * generated from the invocation of the AM_DFG_NODE_PORTS() macro with a set of
 * triplets as its arguments of the form { PORT_NAME, PORT_TYPE, FLAGS
 * }. PROPERTIES is a set of node properties defined by invoking
 * AM_DFG_NODE_PROPERTIES with a list of triplets of the form { PROPERTY_NAME,
 * PROPERTY_HRNAME, TYPE }. PROPERTY_NAME is the name of the property,
 * PROPERTY_HRNAME is the human-redable name of the property and TYPE is the
 * property type as a string.
 *
 * Example:
 * A node type that reads two integers from its input ports a and b and writes
 * the result to its output port c would be defined as follows:
 *
 *   AM_DFG_DECL_NODE_TYPE(am_dfg_node_integer_add,
 *   	"add_integer",
 *   	AM_DFG_NODE_DEFAULT_SIZE,
 *   	AM_DFG_NODE_FUNCTIONS({
 *   		.process = am_dfg_node_integer_process,
 *   		.connect = am_dfg_node_integer_connect_ports
 *   	}),
 *   	AM_DFG_NODE_PORTS(
 *   		{ "a", "int", AM_DFG_PORT_IN | AM_DFG_PORT_MANDATORY },
 *   		{ "b", "int", AM_DFG_PORT_IN | AM_DFG_PORT_MANDATORY },
 *   		{ "c", "int", AM_DFG_PORT_OUT | AM_DFG_PORT_MANDATORY }
 *   	),
 *   	AM_DFG_NODE_PROPERTIES(
 *   		{ "prop1", "A property", "int" },
 *   		{ "prop2", "Another property", "int" },
 *   	)
 */
#define AM_DFG_DECL_BUILTIN_NODE_TYPE(ID, NODE_TYPE_NAME, NODE_TYPE_HRNAME,	\
				      INSTANCE_SIZE, FUNCTIONS, PORTS,		\
				      PROPERTIES)				\
	AM_DFG_DECL_BUILTIN_NODE_TYPE_SWITCH(ID, NODE_TYPE_NAME,		\
					     NODE_TYPE_HRNAME,			\
					     INSTANCE_SIZE, FUNCTIONS,		\
					     PORTS, PROPERTIES)

/* Adds the nodes associated to the identifiers passed as arguments to the list
 * of builtin DFG nodes. There can only be one invocation of
 * AM_DFG_ADD_BUILTIN_NODE_TYPES per header file.
 *
 * Example:
 *
 *   AM_DFG_DECL_BUILTIN_NODE_TYPE(am_dfg_node_integer_add, ...)
 *   AM_DFG_DECL_BUILTIN_NODE_TYPE(am_dfg_node_integer_sub, ...)
 *   AM_DFG_DECL_BUILTIN_NODE_TYPE(am_dfg_node_integer_mul, ...)
 *   AM_DFG_DECL_BUILTIN_NODE_TYPE(am_dfg_node_integer_div, ...)
 *
 *   AM_DFG_ADD_BUILTIN_NODE_TYPES(
 *   		&am_dfg_node_integer_add
 *   		&am_dfg_node_integer_sub
 *   		&am_dfg_node_integer_mul
 *   		&am_dfg_node_integer_div)
 */
#define AM_DFG_ADD_BUILTIN_NODE_TYPES(...) \
	AM_DFG_ADD_BUILTIN_NODE_TYPES_SWITCH(__VA_ARGS__)

#endif
