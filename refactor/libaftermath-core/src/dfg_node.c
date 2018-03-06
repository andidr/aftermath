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

#include <aftermath/core/dfg_node.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* Initialize a port type. The string pointed to by name is copied.*/
int am_dfg_port_type_init(struct am_dfg_port_type* pt,
			  const char* name,
			  const struct am_dfg_type* type,
			  long flags)
{
	if(!(pt->name = strdup(name)))
		return 1;

	pt->flags = flags;
	pt->type = type;

	return 0;
}

/* Destroy a port type */
void am_dfg_port_type_destroy(struct am_dfg_port_type* pt)
{
	free(pt->name);
}

int am_dfg_property_init(struct am_dfg_property* p,
			 const char* name,
			 const char* hrname,
			 const struct am_dfg_type* type)
{
	if(!(p->name = strdup(name)))
		goto out_err;

	if(!(p->hrname = strdup(hrname)))
		goto out_err_name;

	p->type = type;

	return 0;

out_err_name:
	free(p->name);
out_err:
	return 1;
}

void am_dfg_property_destroy(struct am_dfg_property* p)
{
	free(p->name);
	free(p->hrname);
}

/* Destroy a node type */
void am_dfg_node_type_destroy(struct am_dfg_node_type* nt)
{
	free(nt->name);
	free(nt->hrname);

	for(size_t i = 0; i < nt->num_ports; i++)
		am_dfg_port_type_destroy(&nt->ports[i]);

	for(size_t i = 0; i < nt->num_properties; i++)
		am_dfg_property_destroy(&nt->properties[i]);

	free(nt->ports);
	free(nt->properties);
}

/* Connect port other to p. Note that the connection is only one-sided. For a
 * connection in both ways use am_dfg_graph_connectp(). Returns 0 on success,
 * otherwise 1.
 */
int am_dfg_port_connect_onesided(struct am_dfg_port* p,
				 struct am_dfg_port* other)
{
	void* tmp;

	/* Avoid overflow of size_t */
	if(p->num_connections == SIZE_MAX ||
	   p->num_connections + 1 > SIZE_MAX / sizeof(struct am_dfg_port*))
	{
		return 1;
	}

	tmp = realloc(p->connections,
		      (p->num_connections+1)*sizeof(struct am_dfg_port*));

	if(!tmp)
		return 1;

	p->connections = tmp;
	p->connections[p->num_connections] = other;
	p->num_connections++;

	return 0;
}

/*
 * Disconnect other from p. Note that this does not disconnect p from other. To
 * disconnect both ports simultaneously use am_dfg_port_disconnect(). Returns 0 on
 * success, otherwise 1.
 */
int am_dfg_port_disconnect_onesided(struct am_dfg_port* p,
				    struct am_dfg_port* other)
{
	void* tmp;

	am_dfg_buffer_dec_ref(p->buffer);

	for(size_t i = 0; i < p->num_connections; i++) {
		if(p->connections[i] == other) {
			memmove(&p->connections[i],
				&p->connections[i+1],
				(p->num_connections-i-1)*
				sizeof(struct am_dfg_port*));

			p->num_connections--;

			if(p->num_connections == 0) {
				free(p->connections);
				p->connections = NULL;
			} else {
				tmp = realloc(p->connections,
					      p->num_connections*
					      sizeof(struct am_dfg_port*));

				if(!tmp)
					return 1;

				p->connections = tmp;
			}

			return 0;
		}
	}

	return 1;
}

/*
 * Disconnect p_out from p_in and vice versa. Returns 0 on success, otherwise 1.
 */
int am_dfg_port_disconnect(struct am_dfg_port* p_out, struct am_dfg_port* p_in)
{
	if(p_in->node->type->functions.disconnect)
		p_in->node->type->functions.disconnect(p_in->node, p_in);

	if(p_out->node->type->functions.disconnect)
		p_out->node->type->functions.disconnect(p_out->node, p_out);

	return am_dfg_port_disconnect_onesided(p_out, p_in) ||
		am_dfg_port_disconnect_onesided(p_in, p_out);
}

/* Destroy p and free its resources. This does not disconnect any connected
 * port. */
void am_dfg_port_destroy(struct am_dfg_port* p)
{
	free(p->connections);
}

/* Same as am_dfg_node_type_buildv, but does not initialize ports and properties
 * (but allocates the space for them). */
int am_dfg_node_type_build_noinit(struct am_dfg_node_type* nt,
				  struct am_dfg_type_registry* reg,
				  const char* name,
				  const char* hrname,
				  size_t instance_size,
				  size_t num_ports,
				  size_t num_properties)
{
	/* Avoid overflow of size_t */
	if(num_ports > SIZE_MAX / sizeof(struct am_dfg_port_type) ||
	   num_properties > SIZE_MAX / sizeof(struct am_dfg_property))
	{
		goto out_err;
	}

	nt->instance_size = instance_size;
	nt->num_ports = num_ports;
	nt->num_properties = num_properties;
	nt->functions.init = NULL;
	nt->functions.destroy = NULL;
	nt->functions.process = NULL;
	nt->functions.connect = NULL;
	nt->functions.disconnect = NULL;
	nt->functions.to_object_notation = NULL;
	nt->functions.from_object_notation = NULL;
	nt->functions.set_property = NULL;
	nt->functions.get_property = NULL;

	INIT_LIST_HEAD(&nt->list);

	if(!(nt->name = strdup(name)))
		goto out_err;

	if(!(nt->hrname = strdup(hrname)))
		goto out_err_name;

	if(!(nt->ports = malloc(num_ports * sizeof(struct am_dfg_port_type))))
		goto out_err_hrname;

	if(!(nt->properties = malloc(num_properties * sizeof(struct am_dfg_property))))
		goto out_err_ports;

	return 0;

out_err_ports:
	free(nt->ports);
out_err_hrname:
	free(nt->hrname);
out_err_name:
	free(nt->name);
out_err:
	return 1;
}

/*
 * Same as am_dfg_node_type_build, but takes a static definition of a node type.
 */
int am_dfg_node_type_builds(struct am_dfg_node_type* nt,
			    struct am_dfg_type_registry* reg,
			    const struct am_dfg_static_node_type_def* sdef)
{
	struct am_dfg_static_port_type_def* portdef;
	struct am_dfg_static_property_def* propdef;
	const struct am_dfg_type* port_type;
	const struct am_dfg_type* prop_type;
	size_t iports;
	size_t iprops;

	if(am_dfg_node_type_build_noinit(nt, reg, sdef->name,
					 sdef->hrname,
					 sdef->instance_size,
					 sdef->num_ports,
					 sdef->num_properties))
	{
		goto out_err;
	}

	for(iports = 0; iports < sdef->num_ports; iports++) {
		portdef = &sdef->ports[iports];

		if(!(port_type =
		     am_dfg_type_registry_lookup(reg, portdef->type_name)))
		{
			goto out_err_ports;
		}

		if(am_dfg_port_type_init(&nt->ports[iports],
					 portdef->name,
					 port_type,
					 portdef->flags))
		{
			goto out_err_ports;
		}
	}

	for(iprops = 0; iprops < sdef->num_properties; iprops++) {
		propdef = &sdef->properties[iprops];

		if(!(prop_type =
		     am_dfg_type_registry_lookup(reg, propdef->type_name)))
		{
			goto out_err_props;
		}

		if(am_dfg_property_init(&nt->properties[iprops],
					propdef->name,
					propdef->hrname,
					prop_type))
		{
			goto out_err_props;
		}
	}

	nt->functions = sdef->functions;

	return 0;

out_err_props:
	for(size_t j = 0; j < iprops; j++)
		am_dfg_property_destroy(&nt->properties[j]);
out_err_ports:
	for(size_t j = 0; j < iports; j++)
		am_dfg_port_type_destroy(&nt->ports[j]);

	free(nt->ports);
out_err:
	return 1;
}

/*
 * Same as am_dfg_node_type_build, but takes an argument list instead of a
 * sequence of arguments.
 */
int am_dfg_node_type_buildv(struct am_dfg_node_type* nt,
			    struct am_dfg_type_registry* reg,
			    const char* name,
			    const char* hrname,
			    size_t instance_size,
			    size_t num_ports,
			    size_t num_properties,
			    va_list arg)
{
	const char* port_name;
	long port_flags;
	const struct am_dfg_type* port_type;
	const char* port_type_name;
	const char* prop_name;
	const char* prop_hrname;
	const struct am_dfg_type* prop_type;
	const char* prop_type_name;
	size_t iports;
	size_t iprops;

	if(am_dfg_node_type_build_noinit(nt, reg, name, hrname, instance_size,
					 num_ports, num_properties))
	{
		goto out_err;
	}

	for(iports = 0; iports < num_ports; iports++) {
		port_name = va_arg(arg, const char*);
		port_type_name = va_arg(arg, const char*);
		port_flags = va_arg(arg, long);

		if(!(port_type = am_dfg_type_registry_lookup(reg,
							     port_type_name)))
		{
			goto out_err_ports;
		}

		if(am_dfg_port_type_init(&nt->ports[iports],
					 port_name,
					 port_type,
					 port_flags))
		{
			goto out_err_ports;
		}
	}

	for(iprops = 0; iprops < num_properties; iprops++) {
		prop_name = va_arg(arg, const char*);
		prop_hrname = va_arg(arg, const char*);
		prop_type_name = va_arg(arg, const char*);

		if(!(prop_type = am_dfg_type_registry_lookup(reg,
							     prop_type_name)))
		{
			goto out_err_props;
		}

		if(am_dfg_property_init(&nt->properties[iprops],
					prop_name,
					prop_hrname,
					prop_type))
		{
			goto out_err_props;
		}
	}

	return 0;

out_err_props:
	for(size_t j = 0; j < iprops; j++)
		am_dfg_property_destroy(&nt->properties[j]);

	free(nt->properties);
out_err_ports:
	for(size_t j = 0; j < iports; j++)
		am_dfg_port_type_destroy(&nt->ports[j]);

	free(nt->ports);
out_err:
	return 1;
}

/*
 * Register a new node type identified by name. The port types are retrieved
 * from the type registry reg. For each port there must be a triple of arguments
 * const char* port_name, const char* port_type_name, long port_flags.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_node_type_build(struct am_dfg_node_type* nt,
			   struct am_dfg_type_registry* reg,
			   const char* name,
			   const char* hrname,
			   size_t instance_size,
			   size_t num_ports,
			   size_t num_properties,
			   ...)
{
	va_list arg;
	int ret;

	va_start(arg, num_properties);
	ret = am_dfg_node_type_buildv(nt, reg, name, hrname, instance_size,
				      num_ports, num_properties, arg);
	va_end(arg);

	return ret;
}

/*
 * Allocate a new instance of the node type t.
 *
 * Returns an uninitialized node or NULL if the allocation fails.
 */
struct am_dfg_node* am_dfg_node_alloc(const struct am_dfg_node_type* t)
{
	return malloc(t->instance_size);
}

/*
 * Create a new instance of the node type t and assign it to n. If id is zero, a
 * new unique id is generated from the address n.
 *
 * Return 0 on success, 1 otherwise.
 */
int am_dfg_node_instantiate(struct am_dfg_node* n,
			    const struct am_dfg_node_type* t,
			    long id)
{
	n->type = t;
	n->id = id;

	if(id == 0)
		n->id = (long)n;

	/* Avoid overflow of size_t */
	if(t->num_ports > SIZE_MAX / sizeof(struct am_dfg_port))
		goto out_err;

	if(!(n->ports = malloc(t->num_ports * sizeof(struct am_dfg_port))))
		goto out_err;

	for(size_t i = 0; i < t->num_ports; i++) {
		n->ports[i].type = &t->ports[i];
		n->ports[i].buffer = NULL;
		n->ports[i].connections = NULL;
		n->ports[i].num_connections = 0;
		n->ports[i].node = n;
	}

	if(n->type->functions.init)
		if(n->type->functions.init(n))
			goto out_err_init;

	return 0;

out_err_init:
	free(n->ports);
out_err:
	return 1;
}

/*
 * Return the index of p in n->ports. If n->ports does not contain p, -1 is
 * returned.
 */
size_t am_dfg_node_get_port_index(const struct am_dfg_node* n,
				  const struct am_dfg_port* p)
{
	size_t idx = 0;
	enum am_dfg_port_flag type;
	const struct am_dfg_port* pi;

	if(p->type->flags & AM_DFG_PORT_IN)
		type = AM_DFG_PORT_IN;
	else
		type = AM_DFG_PORT_OUT;

	am_dfg_node_for_each_port(n, pi) {
		if(pi == p)
			return idx;

		if(pi->type->flags & type)
			idx++;
	}

	return -1;
}

/*
 * Checks if n is a root, i.e., a node that does not have any incoming
 * connection. Returns true if n is a root, otherwise false.
 */
int am_dfg_node_is_root(const struct am_dfg_node* n)
{
	struct am_dfg_port* p;

	am_dfg_node_for_each_port(n, p)
		if(p->type->flags & AM_DFG_PORT_IN)
			if(p->num_connections > 0)
				return 0;

	return 1;
}

/*
 * Checks if n is a root, i.e., a node that does not have any incoming
 * connection besides the connection indicated by ignore_src and
 * ignore_dst. Returns true if n is a root according to this definition,
 * otherwise false.
 */
int am_dfg_node_is_root_ign(const struct am_dfg_node* n,
			    const struct am_dfg_port* ignore_src,
			    const struct am_dfg_port* ignore_dst)
{
	struct am_dfg_port* p;

	am_dfg_node_for_each_input_port(n, p) {
		if(p->num_connections > 1 ||
		   (p->num_connections == 1 &&
		    p == ignore_dst &&
		    p->connections[0] != ignore_src))
		{
			return 0;
		}
	}

	return 1;
}

/*
 * Resets the number of remaining dependencies and the marking of n.
 */
void am_dfg_node_reset_sched_data(struct am_dfg_node* n)
{
	struct am_dfg_port* p;

	n->num_deps_remaining = 0;
	n->marking = 0;

	am_dfg_node_for_each_port(n, p) {
		if(p->type->flags & AM_DFG_PORT_IN) {
			if(p->type->flags & AM_DFG_PORT_MANDATORY ||
			   am_dfg_port_is_connected(p))
			{
				n->num_deps_remaining++;
			}
		}
	}
}

/*
 * Find the port of n with a specific name. If n does not have a port with the
 * indicated name, the function returns NULL.
 */
struct am_dfg_port* am_dfg_node_find_port(const struct am_dfg_node* n,
					  const char* name)
{
	struct am_dfg_port* p;

	am_dfg_node_for_each_port(n, p)
		if(strcmp(p->type->name, name) == 0)
			return p;

	return NULL;
}

/*
 * Checks if all mandatory ports of n are connected.
 */
int am_dfg_node_is_well_connected(const struct am_dfg_node* n)
{
	struct am_dfg_port* p;

	am_dfg_node_for_each_port(n, p) {
		if(p->num_connections == 0 &&
		   p->type->flags & AM_DFG_PORT_MANDATORY)
		{
			return 0;
		}
	}

	return 1;
}

/*
 * Destroy a node n, including all of its ports.
 */
void am_dfg_node_destroy(struct am_dfg_node* n)
{
	struct am_dfg_port* p;

	if(n->type->functions.destroy)
		n->type->functions.destroy(n);

	am_dfg_node_for_each_port(n, p)
		am_dfg_port_destroy(p);

	free(n->ports);
}

/* Convert a node into its representation in object notation. Returns a newly
 * created object notation node of the form:
 *
 *    <type> {
 *         id: <integer>;
 *    }
 *
 * In case of an error NULL is returned.
 */
struct am_object_notation_node*
am_dfg_node_to_object_notation(struct am_dfg_node* n)
{
	struct am_object_notation_node_group* g;
	struct am_object_notation_node* ng;

	ng = am_object_notation_build(
		AM_OBJECT_NOTATION_BUILD_GROUP, n->type->name,
		  AM_OBJECT_NOTATION_BUILD_MEMBER, "id",
		    AM_OBJECT_NOTATION_BUILD_INT, (int64_t)n->id,
		AM_OBJECT_NOTATION_BUILD_END);

	if(!ng)
		goto out_err;

	g = (struct am_object_notation_node_group*)ng;

	if(n->type->functions.to_object_notation)
		if(n->type->functions.to_object_notation(n, g))
			goto out_err_dest;

	return ng;

out_err_dest:
	am_object_notation_node_destroy(ng);
	free(ng);
out_err:
	return NULL;
}

/* Builds a DFG node of type nt from an object notation node n_node. Returns the
 * newly allocated and initialized node on success, otherwise NULL. */
struct am_dfg_node*
am_dfg_node_from_object_notation(struct am_dfg_node_type* nt,
				 struct am_object_notation_node_group* g)
{
	uint64_t u64id;
	long id;

	struct am_dfg_node* ret = NULL;

	if(am_object_notation_eval_retrieve_int(&g->node, "id", &u64id))
		goto out_err;

	if(u64id > LONG_MAX)
		goto out_err;

	id = u64id;

	if(!(ret = am_dfg_node_alloc(nt)))
		goto out_err;

	if(am_dfg_node_instantiate(ret, nt, id))
		goto out_err_free;

	if(nt->functions.from_object_notation)
		if(nt->functions.from_object_notation(ret, g))
			goto out_err_dest;

	return ret;

out_err_dest:
	am_dfg_node_destroy(ret);
out_err_free:
	free(ret);
out_err:
	return NULL;
}
