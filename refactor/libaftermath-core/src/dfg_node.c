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

	am_dfg_port_mask_reset(&pt->new_mask);
	am_dfg_port_mask_reset(&pt->old_mask);

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

	/* When (re)connecting an input port, the generation must be reset, such
	 * that the generation of the output port the input port was last
	 * connected to is not taken for the last generation of the new output
	 * port. */
	if(am_dfg_port_is_input_port(p))
		p->generation = 0;

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
	const struct am_dfg_node_type_functions* in_funs;
	const struct am_dfg_node_type_functions* out_funs;

	in_funs = &p_in->node->type->functions;
	out_funs = &p_out->node->type->functions;

	if(in_funs->disconnect)
		in_funs->disconnect(p_in->node, p_in);

	if(out_funs->disconnect)
		out_funs->disconnect(p_out->node, p_out);

	return am_dfg_port_disconnect_onesided(p_out, p_in) ||
		am_dfg_port_disconnect_onesided(p_in, p_out);
}

/* Destroy p and free its resources. This does not disconnect any connected
 * port. */
void am_dfg_port_destroy(struct am_dfg_port* p)
{
	free(p->connections);
}

/* Returns a pointer to the port type of a node type nt with the name specified
 * in port_name. If no port type has the specified name, the function returns
 * NULL.
 */
struct am_dfg_port_type*
am_dfg_node_type_find_port_type(const struct am_dfg_node_type* nt,
				const char* port_name)
{
	struct am_dfg_port_type* pt;

	/* FIXME: speed up linear look-up */
	am_dfg_node_type_for_each_port_type(nt, pt)
		if(strcmp(pt->name, port_name) == 0)
			return pt;

	return NULL;
}

/* Generates a bit mask from a NULL-terminated list of port names for a node
 * type. The bits set to 1 are at indexes that correspond to the indexes of the
 * ports with the specified names within the array of ports of the node type.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_node_type_build_node_dep_mask(const struct am_dfg_node_type* nt,
					 uint64_t* mask,
					 const char** port_names)
{
	const struct am_dfg_port_type* pt;
	uint64_t retmask;
	size_t bpos;

	retmask = 0;

	if(!port_names)
		return 0;

	for(size_t i = 0; port_names[i] != NULL; i++) {
		if(!(pt = am_dfg_node_type_find_port_type(nt, port_names[i])))
			return 1;

		/* Current limit is 64 ports */
		if((bpos = AM_ARRAY_INDEX(nt->ports, pt)) >= 64)
			return 1;

		retmask |= UINT64_C(1) << bpos;
	}

	*mask = retmask;

	return 0;
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

/* Initializes the port dependence masks of a port type */
static void am_dfg_static_port_def_init_masks(
	const struct am_dfg_static_port_type_def* sptd,
	struct am_dfg_port_type* pt,
	const struct am_dfg_node_type* nt)
{
	am_dfg_port_mask_reset(&pt->new_mask);
	am_dfg_port_mask_reset(&pt->old_mask);

	/* Add self reference for output ports, such that if a consumer pulls a
	 * new value, the output port by default indicates that only old data is
	 * available. This triggers application of the "old value" mask of the
	 * output port, which usually triggers inclusion of input ports into the
	 * "pull new" mask. */
	if(am_dfg_port_type_is_output_type(pt))
		pt->new_mask.push_old |= am_dfg_port_type_mask_bits(pt, nt);
}

/* Adds port dependencies for all ports of nt according to the value
 * AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL. See documentation of that value for
 * the effect.
 */
static void
am_dfg_node_type_set_portdeps_pure_functional(struct am_dfg_node_type* nt)
{
	struct am_dfg_port_type* pt;
	uint64_t in_ports = am_dfg_node_type_input_mask(nt);
	uint64_t out_ports = am_dfg_node_type_output_mask(nt);

	am_dfg_node_type_for_each_port_type(nt, pt) {
		if(am_dfg_port_type_is_input_type(pt)) {
			/* If new data is available at this port, indicate that
			 * all ouput ports will provide new data. Also, indicate
			 * that data will be needed to on all input ports
			 * regardless of the age, since we assume that the node
			 * does not cache any input values. */
			pt->new_mask.push_new |= out_ports;
			pt->new_mask.push_old |= 0;
			pt->new_mask.pull_old |= in_ports;
			pt->new_mask.pull_new |= 0;

			/* If old data is available at this port, indicate that
			 * all ouput ports will also provide old data. Input
			 * ports will be pulled for new data. */
			pt->old_mask.push_new |= 0;
			pt->old_mask.push_old |= out_ports;
			pt->old_mask.pull_old |= 0;
			pt->old_mask.pull_new |= in_ports;
		} else {
			/* If a connected input port asks for new data, indicate
			 * that no new data is available and ask on all input
			 * ports for new data. */
			pt->new_mask.push_new |= 0;
			pt->new_mask.push_old |= out_ports;
			pt->new_mask.pull_old |= 0;
			pt->new_mask.pull_new |= in_ports;

			/* If a connected input port asks for data regardless of
			 * the age, indicate that old data will be available at
			 * each output port and force pulling in data regardless
			 * of the age on all input ports. */
			pt->old_mask.push_new |= 0;
			pt->old_mask.push_old |= out_ports;
			pt->old_mask.pull_old |= in_ports;
			pt->old_mask.pull_new |= 0;
		}
	}
}

/* Adds port dependencies to the port definitions of a node type nt according to
 * the specified default dependencies of a static node type definition sdef. */
static void am_dfg_node_type_build_default_masks(
	struct am_dfg_node_type* nt,
	const struct am_dfg_static_node_type_def* sdef)
{
	switch(sdef->default_port_deps) {
		case AM_DFG_DEFAULT_PORT_DEPS_NONE:
			break;
		case AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL:
			am_dfg_node_type_set_portdeps_pure_functional(nt);
			break;
	}
}

/* FIXME: This could entirely be done at compile time. This would require some
 * macro trickery for AM_DFG_DECL_BUILTIN_NODE_TYPE and AM_DFG_NODE_PORTS or
 * some external preprocessor generating the masks.
 */

/* Adds port dependencies to the port definitions of a node type nt according to
 * the specified explicit per-port dependencies of a static node type definition
 * sdef. */
static int am_dfg_node_type_build_explicit_masks(
	struct am_dfg_node_type* nt,
	const struct am_dfg_static_node_type_def* sdef)
{
	struct am_dfg_static_port_type_def* portdef;
	struct am_dfg_static_port_dep_word* portdep;
	struct am_dfg_port_mask* mask = NULL;
	struct am_dfg_port_type* pt;
	struct am_dfg_port_type* ptother;
	uint64_t* mask_field = NULL;
	const char** pname;
	uint64_t pbits;

	/* Process explicit port dependencies */
	for(size_t i = 0; i < nt->num_ports; i++) {
		portdef = &sdef->ports[i];
		pt = &nt->ports[i];

		am_dfg_static_port_def_init_masks(portdef, pt, nt);
	}

	for(size_t i = 0; i < sdef->num_port_deps; i++) {
		portdep = &sdef->port_deps[i];

		if(!(pt = am_dfg_node_type_find_port_type(nt, portdep->trigger_port)))
			return 1;

		switch(portdep->trigger) {
			case AM_DFG_PORT_DEP_ON_NEW:
				mask = &pt->new_mask;
				break;
			case AM_DFG_PORT_DEP_ON_OLD:
				mask = &pt->old_mask;
				break;
		}

		switch(portdep->reaction) {
			case AM_DFG_PORT_DEP_PULL_NEW:
				mask_field = &mask->pull_new;
				break;
			case AM_DFG_PORT_DEP_PULL_OLD:
				mask_field = &mask->pull_old;
				break;
			case AM_DFG_PORT_DEP_PUSH_NEW:
				mask_field = &mask->push_new;
				break;
			case AM_DFG_PORT_DEP_PUSH_OLD:
				mask_field = &mask->push_old;
				break;
		}

		for(pname = &portdep->reaction_ports[0]; *pname; pname++) {
			if(!(ptother = am_dfg_node_type_find_port_type(nt, *pname)))
				return 1;

			pbits = am_dfg_port_type_mask_bits(ptother, nt);
			*mask_field |= pbits;
		}
	}

	return 0;
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

	nt->functions = sdef->functions;

	if(am_dfg_node_type_build_explicit_masks(nt, sdef))
		goto out_err_ports;

	am_dfg_node_type_build_default_masks(nt, sdef);

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

	if(!(n->ports = calloc(t->num_ports, sizeof(struct am_dfg_port))))
		goto out_err;

	am_dfg_port_mask_reset(&n->negotiated_mask);
	am_dfg_port_mask_reset(&n->required_mask);
	am_dfg_port_mask_reset(&n->propagated_mask);

	for(size_t i = 0; i < t->num_ports; i++) {
		n->ports[i].type = &t->ports[i];
		n->ports[i].buffer = NULL;
		n->ports[i].connections = NULL;
		n->ports[i].num_connections = 0;
		n->ports[i].node = n;

		/* Set the generation of output data to 1 and the generation of
		 * input data to 0, such that a newly created input port
		 * connecting to an output port 'sees' new data, even if pulling
		 * in new mode. */
		if(am_dfg_port_type_is_output_type(n->ports[i].type))
			n->ports[i].generation = 1;
		else
			n->ports[i].generation = 0;
	}

	INIT_LIST_HEAD(&n->sched_list);

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
