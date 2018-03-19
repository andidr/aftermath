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

#include <aftermath/core/dfg_schedule.h>
#include <aftermath/core/safe_alloc.h>
#include <aftermath/core/bits.h>

enum am_dfg_schedule_marking {
	/* No marking */
	AM_DFG_SCHEDULE_MARK_NONE,

	/* Node has been reset */
	AM_DFG_SCHEDULE_MARK_RESET,

	/* Indicates that the node is currently being taken into account for a
	 * marking of a subgraph. This marking may only be used during the
	 * marking process. */
	AM_DFG_SCHEDULE_MARK_MARKING,

	/* Node already evaluated during search for roots */
	AM_DFG_SCHEDULE_MARK_FIND_ROOTS,

	/* Consistency check from the scheduler on the node prior to actual
	 * scheduling is currently being carried out */
	AM_DFG_SCHEDULE_MARK_CHECKING,

	/* Node has not (yet) been processed */
	AM_DFG_SCHEDULE_MARK_UNPROCESSED,

	/* Node is currently being considered for processing */
	AM_DFG_SCHEDULE_MARK_PROCESSING,

	/* Node has been processed */
	AM_DFG_SCHEDULE_MARK_DONE
};

/*
 * Marks all the nodes of the connected component that a node n belongs to with
 * the marking m.
 *
 * Returns 0 on success, otherwise 1.
 */
static void am_dfg_schedule_mark_component(struct am_dfg_node* n,
					   enum am_dfg_schedule_marking m)
{
	struct am_dfg_port* p;
	struct am_dfg_port* pother;
	size_t i;

	/* Sub-graph already considered? */
	if(n->marking == AM_DFG_SCHEDULE_MARK_MARKING)
		return;

	n->marking = AM_DFG_SCHEDULE_MARK_MARKING;

	am_dfg_node_for_each_port(n, p)
		am_dfg_port_for_each_connected_port_safe(p, pother, i)
			am_dfg_schedule_mark_component(pother->node, m);


	n->marking = m;
}

/* Resets a node and all of its ports and buffers */
void am_dfg_schedule_reset_node(struct am_dfg_node* n)
{
	struct am_dfg_port* p;

	INIT_LIST_HEAD(&n->sched_list);
	n->num_deps_remaining = 0;

	am_dfg_port_mask_reset(&n->negotiated_mask);
	am_dfg_port_mask_reset(&n->propagated_mask);

	am_dfg_node_for_each_output_port(n, p)
		if(p->buffer)
			am_dfg_buffer_reset(p->buffer);

	n->marking = AM_DFG_SCHEDULE_MARK_RESET;
}

/* Removes a node from its current list and inserts it at the front of a list
 * l. */
static inline void am_dfg_schedule_list_push_front(struct list_head* l,
						   struct am_dfg_node* n)
{
	list_del(&n->sched_list);
	list_add(&n->sched_list, l);
}

/* For debugging: Dumps a list of nodes to be scheduled to stdout */
void am_dfg_schedule_list_dump(struct list_head* l)
{
	struct am_dfg_node* n;

	printf("Scheduling list %p:\n", l);
	am_typed_list_for_each_genentry(l, n, sched_list)
		printf("  Node %p [%s]\n", n, n->type->name);
}

/* Removes the first entry from a list l and returns the associated node. If the
 * list is empty, the function returns NULL. */
static inline struct am_dfg_node*
am_dfg_schedule_list_pop_front(struct list_head* l)
{
	struct am_dfg_node* ret = NULL;

	if(!list_empty(l)) {
		ret = list_first_entry(l, typeof(*ret), sched_list);
		list_del(&ret->sched_list);
		INIT_LIST_HEAD(&ret->sched_list);
	}

	return ret;
}

/* Defines a new function that propagates the information required ports of a
 * node to the neighbors depending on the port direction (input or output) and
 * the mode for the port (new or always). If the mask for a neighboring node
 * changes, it is added to the list of nodes to be re-evaluated.
 */
#define AM_DFG_PORT_DECL_PROPAGATE_FUN(suffix, port_type, others_mask_name)	\
	static inline int am_dfg_port_propagate_##suffix(			\
		const struct am_dfg_node* n, uint64_t ports,			\
		struct list_head* sched_list)					\
	{									\
		struct am_dfg_port* pother;					\
		struct am_dfg_port* p;						\
		struct am_dfg_node* nother;					\
		const struct am_dfg_port_type* ptother;			\
		uint64_t tmp;							\
		size_t pidx;							\
		size_t i;							\
										\
		am_for_each_bit_idx_u64(ports, tmp, pidx) {			\
			p = &n->ports[pidx];					\
										\
			if(!am_dfg_port_is_##port_type##_port(p))		\
				return 1;					\
										\
			am_dfg_port_for_each_connected_port_safe(p, pother, i) {\
				nother = pother->node;				\
				ptother = pother->type;			\
										\
				if(!am_dfg_port_mask_is_subset(		\
					   &nother->negotiated_mask,		\
					   &ptother->others_mask_name))	\
				{						\
					am_dfg_port_mask_apply(		\
						&nother->negotiated_mask,	\
						&ptother->others_mask_name);	\
										\
					am_dfg_schedule_list_push_front(	\
						sched_list, nother);		\
				}						\
			}							\
		}								\
										\
		return 0;							\
	}

AM_DFG_PORT_DECL_PROPAGATE_FUN(pulled_if_new_inputs_to_producer,
			       input,
			       new_mask)

AM_DFG_PORT_DECL_PROPAGATE_FUN(pulled_always_inputs_to_producer,
			       input,
			       old_mask)

AM_DFG_PORT_DECL_PROPAGATE_FUN(pushed_new_outputs_to_consumers,
			       output,
			       new_mask)

AM_DFG_PORT_DECL_PROPAGATE_FUN(pushed_same_outputs_to_consumers,
			       output,
			       old_mask)

/* Evaluates the dependencies of a node n. Producer or consumer nodes of n for
 * which the ingoing or outgoing dependencies have changed and that need to be
 * re-evaluated are added to the list sched_list.
 *
 * Returns 0 on success, 1 otherwise.
 */
static int
am_dfg_schedule_node_eval_dependencies(struct am_dfg_node* n,
				       struct list_head* sched_list)
{
	struct am_dfg_port* p;
	struct am_dfg_port_mask dmask;
	uint64_t in_mask;
	uint64_t tmp;

	/* Determine which changes since the last evaluation need to be
	 * propagated */
	am_dfg_port_mask_diff(&dmask, &n->negotiated_mask, &n->propagated_mask);

	/* Propagate the changes to neighbors */
	if(am_dfg_port_propagate_pulled_if_new_inputs_to_producer(
		   n, dmask.pull_new, sched_list) ||
	   am_dfg_port_propagate_pulled_always_inputs_to_producer(
		   n, dmask.pull_old, sched_list) ||
	   am_dfg_port_propagate_pushed_new_outputs_to_consumers(
		   n, dmask.push_new, sched_list) ||
	   am_dfg_port_propagate_pushed_same_outputs_to_consumers(
		   n, dmask.push_old, sched_list))
	{
		return 1;
	}

	/* Update information on propagated changes */
	am_dfg_port_mask_apply(&n->propagated_mask, &n->negotiated_mask);

	/* Update number of incoming dependencies */
	in_mask = n->negotiated_mask.pull_old |
		n->negotiated_mask.pull_new;

	am_dfg_node_for_each_masked_port(n, in_mask, p, tmp)
		if(!am_dfg_port_activated(p))
			in_mask &= ~am_dfg_port_mask_bits(p);

	n->num_deps_remaining = am_num_bits_set_u64(in_mask);

	return 0;
}

/* Returns 1 if a node n does not depend on data of any other node. */
static inline int am_dfg_schedule_is_root(struct am_dfg_node* n)
{
	struct am_dfg_port* p;
	uint64_t out_mask;
	uint64_t tmp;

	out_mask = n->negotiated_mask.push_old | n->negotiated_mask.push_new;

	if(n->num_deps_remaining != 0)
		return 0;

	/* A root must have at least one consumer */
	am_dfg_node_for_each_masked_port(n, out_mask, p, tmp)
		if(am_dfg_port_activated(p))
			return 1;

	return 0;
}

/* Finds a ll the roots in the connected component to which n belongs and adds
 * the to the list l. */
static void __am_dfg_schedule_component_find_roots(struct am_dfg_node* n,
						   struct list_head* l)
{
	struct am_dfg_port* p;
	struct am_dfg_port* pother;
	uint64_t used_mask;
	uint64_t tmp;
	size_t i;

	/* Sub-graph already checked? */
	if(n->marking == AM_DFG_SCHEDULE_MARK_FIND_ROOTS)
		return;

	n->marking = AM_DFG_SCHEDULE_MARK_FIND_ROOTS;

	if(am_dfg_schedule_is_root(n))
		list_add(&n->sched_list, l);

	used_mask = n->negotiated_mask.push_old |
		n->negotiated_mask.push_new |
		n->negotiated_mask.pull_new |
		n->negotiated_mask.pull_old;

	am_dfg_node_for_each_masked_port(n, used_mask, p, tmp)
		am_dfg_port_for_each_connected_port_safe(p, pother, i)
			__am_dfg_schedule_component_find_roots(pother->node, l);
}

/* Wrapper for __am_dfg_schedule_component_find_roots that resets the marking of
 * the component n belongs to before locating roots. */
static void am_dfg_schedule_component_find_roots(struct am_dfg_node* n,
						 struct list_head* l)
{
	am_dfg_schedule_mark_component(n, AM_DFG_SCHEDULE_MARK_NONE);
	__am_dfg_schedule_component_find_roots(n, l);
}

/* Resets all the nodes of the component to which n belongs. Prior to the
 * initial call to am_dfg_schedule_reset_component (i.e., the call on the
 * initial node), none of the nodes must have the marking
 * AM_DFG_SCHEDULE_MARK_RESET. Otherwise, it is not guaranteed that the entire
 * component is marked.
 */
static void am_dfg_schedule_reset_component(struct am_dfg_node* n)
{
	struct am_dfg_port* p;
	struct am_dfg_port* pother;
	size_t i;

	/* Sub-graph already done? */
	if(n->marking == AM_DFG_SCHEDULE_MARK_RESET)
		return;

	am_dfg_schedule_reset_node(n);

	am_dfg_node_for_each_port(n, p)
		am_dfg_port_for_each_connected_port_safe(p, pother, i)
			am_dfg_schedule_reset_component(pother->node);
}

/* For debugging: Dumps the status of a node and alls of its ports to stdout. */
void am_dfg_schedule_dump_node(struct am_dfg_node* n)
{
	struct am_dfg_port* p;
	uint64_t pmask;

	printf("Node %ld [%s]\n", n->id, n->type->name);

	am_dfg_node_for_each_port(n, p) {
		pmask = am_dfg_port_mask_bits(p);

		printf("  Port %s [%s]: "
		       "Connected: %s, "
		       "Activated: %s, "
		       "Pull new: %s, "
		       "Pull old: %s, "
		       "Push new: %s, "
		       "Push old: %s\n",
		       p->type->name,
		       am_dfg_port_is_input_port(p) ? "in" : "out",
		       am_dfg_port_is_connected(p) ? "Y" : "N",
		       am_dfg_port_activated(p) ? "Y" : "N",
		       (n->negotiated_mask.pull_new & pmask) ? "Y" : "N",
		       (n->negotiated_mask.pull_old & pmask) ? "Y" : "N",
		       (n->negotiated_mask.push_new & pmask) ? "Y" : "N",
		       (n->negotiated_mask.push_old & pmask) ? "Y" : "N");
	}
}

/* Processes a single node. The node must be ready for execution, i.e., all
 * input dependencies must be satisfied. Nodes whose only remaining input
 * dependency is an outgoing edge of this node are added to sched_list.
 *
 * Nodes with output ports that have all been marked as not producing new data
 * and which are requested by the consumers only if they produce new data are
 * omitted. If the node does not have any output ports that are pulled
 * regardless of the age of the data, execution of the node is omitted.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_schedule_process_node(struct am_dfg_node* n,
				 struct list_head* sched_list)
{
	struct am_dfg_port* p;
	struct am_dfg_port* pother;
	uint64_t in_mask;
	uint64_t out_mask;
	uint64_t req_mask;
	uint64_t tmp;
	size_t i;
	uint64_t omittable_ports;
	int skip_execution;

	if(n->num_deps_remaining != 0)
		return 1;

	n->marking = AM_DFG_SCHEDULE_MARK_PROCESSING;

	/* Update the data generation of each output port markes as new */
	am_dfg_node_for_each_masked_port(n, n->negotiated_mask.push_new, p, tmp)
		am_dfg_output_port_inc_generation(p);

	out_mask = n->negotiated_mask.push_new | n->negotiated_mask.push_old;
	in_mask = n->negotiated_mask.pull_new | n->negotiated_mask.pull_old;
	req_mask = out_mask | in_mask;
	omittable_ports = 0;

	am_dfg_node_for_each_masked_port(n, req_mask, p, tmp) {
		if(!am_dfg_port_activated(p))
			omittable_ports |= am_dfg_port_mask_bits(p);
	}

	skip_execution = (req_mask == omittable_ports);

	if(!skip_execution && n->type->functions.process)
		if(n->type->functions.process(n))
			return 1;

	/* Update generation of all connected and requested input ports */
	am_dfg_node_for_each_masked_port(n, in_mask, p, tmp)
		if(am_dfg_port_is_connected(p))
			p->generation = p->connections[0]->generation;

	/* Update synchronization counter of consumers */
	am_dfg_node_for_each_masked_port(n, out_mask, p, tmp) {
		am_dfg_port_for_each_connected_port_safe(p, pother, i) {
			if(am_dfg_port_activated(pother)) {
				/* If the synchronization counter is already 0
				 * something must have gone wrong.
				 *
				 * FIXME: rather use assert() for these kinds
				 * of situations */
				if(pother->node->num_deps_remaining == 0)
					return 1;

				if(--pother->node->num_deps_remaining == 0) {
					list_add(&pother->node->sched_list,
						 sched_list);
				}
			}
		}
	}

	n->marking = AM_DFG_SCHEDULE_MARK_DONE;

	return 0;
}

/* Resets all the nodes of a graph g. */
void am_dfg_schedule_reset_graph(const struct am_dfg_graph* g)
{
	struct am_dfg_node* n;

	am_dfg_graph_for_each_node(g, n)
		am_dfg_schedule_reset_node(n);
}

/* Propagates the changes of the negotiated mask of all nodes included in the
 * list until the negotiated masks of the nodes reachable from any of the nodes
 * in the list converge (i.e., they do not change anymore).
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_schedule_converge_deps(struct list_head* sched_list)
{
	struct am_dfg_node* niter;

	while((niter = am_dfg_schedule_list_pop_front(sched_list)))
		if(am_dfg_schedule_node_eval_dependencies(niter, sched_list))
			return 1;

	return 0;
}

/* Schedules all the nodes in a list as well as all of their descendants when
 * these get activated. All nodes in sched_list must be scheduling roots.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_schedule_nodes(struct list_head* sched_list)
{
	struct am_dfg_node* niter;

	while((niter = am_dfg_schedule_list_pop_front(sched_list)))
		if(am_dfg_schedule_process_node(niter, sched_list))
			return 1;

	return 0;
}

/* Attempts to schedule all the nodes of a graph g by setting all input ports to
 * "input old", such that they request data even if the producer does not
 * provide any new data. Nothe that this does not necessarily mean that all
 * nodes are executed, since the dependence masks of a node might cause the node
 * not produce any data, even if requested on the node's output ports.
 *
 * Returns 0 on sucess, otherwise 1.
 */
int am_dfg_schedule_graph(const struct am_dfg_graph* g)
{
	struct am_dfg_node* n;
	struct list_head sched_list = LIST_HEAD_INIT(sched_list);

	am_dfg_schedule_reset_graph(g);

	/* Mask all input ports as "pull always" */
	am_dfg_graph_for_each_node(g, n) {
		am_dfg_port_mask_reset(&n->required_mask);
		n->required_mask.pull_old = am_dfg_node_type_input_mask(n->type);
		am_dfg_port_mask_copy(&n->negotiated_mask, &n->required_mask);
		am_dfg_schedule_list_push_front(&sched_list, n);
	}

	if(am_dfg_schedule_converge_deps(&sched_list))
		return 1;

	am_dfg_graph_for_each_node(g, n)
		if(n->num_deps_remaining == 0)
			am_dfg_schedule_list_push_front(&sched_list, n);


	if(am_dfg_schedule_nodes(&sched_list))
		return 1;

	return 0;
}

/* Tries to schedule n. Masks for the minimum input and output dependencies must
 * be set prior to the call. Requirements are propagated within the connected
 * component that, belongs to, including additional requirements of n itself
 * that arise from requirements of its consumers and producers.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_schedule_component(struct am_dfg_node* n)
{
	struct list_head sched_list = LIST_HEAD_INIT(sched_list);

	am_dfg_schedule_mark_component(n, AM_DFG_SCHEDULE_MARK_NONE);
	am_dfg_schedule_reset_component(n);

	am_dfg_port_mask_copy(&n->negotiated_mask, &n->required_mask);

	am_dfg_schedule_list_push_front(&sched_list, n);

	if(am_dfg_schedule_converge_deps(&sched_list))
		return 1;

	am_dfg_schedule_component_find_roots(n, &sched_list);

	/* FIXME: If nothing is schedules, this isn't necessarily an error (e.g.,
	 * if n only produces data and none of its consumers pulls data)
	 */
	if(list_empty(&sched_list))
		return 1;

	if(am_dfg_schedule_nodes(&sched_list))
		return 1;

	return 0;
}
