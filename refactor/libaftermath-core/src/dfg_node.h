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
};

/* A node type */
struct am_dfg_node_type {
	/* The name of this node type */
	char* name;

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
};

#define am_dfg_node_for_each_port(n, p)		\
	for((p) = &(n)->ports[0];			\
	    (p) != &(n)->ports[(n)->type->num_ports];	\
	    (p)++)

void am_dfg_node_type_destroy(struct am_dfg_node_type* nt);

int am_dfg_node_type_buildv(struct am_dfg_node_type* nt,
			    struct am_dfg_type_registry* reg,
			    const char* name,
			    size_t instance_size,
			    size_t num_ports,
			    va_list arg);

int am_dfg_node_type_build(struct am_dfg_node_type* nt,
			   struct am_dfg_type_registry* reg,
			   const char* name,
			   size_t instance_size,
			   size_t num_ports,
			   ...);

/* Instance of a node */
struct am_dfg_node {
	/* The node's node type */
	const struct am_dfg_node_type* type;

	/* The node's port instances */
	struct am_dfg_port* ports;

	/* List to chain all nodes (e.g., nodes of a graph) */
	struct list_head list;

	/* Number of remaining dependencies (used for scheduling) */
	size_t num_deps_remaining;

	/* Marking used by the scheduler / cycle checker */
	int marking;

	/* The id of this node instance */
	long id;
};

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

#endif
