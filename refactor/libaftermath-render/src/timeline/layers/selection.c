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

#include <aftermath/render/timeline/layers/selection.h>
#include <aftermath/render/timeline/renderer.h>
#include <aftermath/render/cairo_extras.h>

#include "selection_array_defs.h"

#define SELECTION_ACC_END(x) ((x).interval.start)

/* Comparison expression for sorting selections in descending order of their
 * end */
#define SELECTION_CMP_REV_END(a, b) AM_VALCMP_EXPR(a, b)

AM_DECL_TYPED_ARRAY_INSERTPOS(
	am_timeline_selection_array,
	struct am_timeline_selection,
	am_timestamp_t,
	SELECTION_ACC_END,
	SELECTION_CMP_REV_END);

AM_DECL_TYPED_ARRAY_RESERVE_SORTED(
	am_timeline_selection_array,
	struct am_timeline_selection,
	am_timestamp_t)

AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(
	am_timeline_selection_array,
	struct am_timeline_selection,
	interval)

AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_LAST_OVERLAPPING(
	am_timeline_selection_array,
	struct am_timeline_selection,
	interval)

struct selection_array_iter_bounds {
	struct am_timeline_selection* start;
	struct am_timeline_selection* end;
};

/* Iterates iter over all selections of sl whose intervals overlap with the
 * interval *pqi. */
#define am_timeline_selection_array_for_each_overlapping(sl, pqi, iter)	\
	for(struct selection_array_iter_bounds __iter = {			\
	    	.start = ((iter) =						\
	    		  am_timeline_selection_array_bsearch_first_overlapping(\
	    			  &(sl)->selections, (pqi))),			\
	    	.end = am_timeline_selection_array_bsearch_last_overlapping(	\
	    		&(sl)->selections, (pqi))				\
	    };									\
	    (iter) && AM_PTR_LEQ((iter), __iter.end);				\
	    (iter)++)

/* Parameters for a single start/end line */
struct line_param {
	struct am_rgba color;
	double width;
};

/* Parameters for the rectangle between the start and end */
struct interval_param {
	struct am_rgba color;
};

struct selection_layer_params {
	/* Color and width for the vertical line indicating the start of a
	 * selection */
	struct line_param start_line;

	/* Color and width for the vertical line indicating the end of a
	 * selection */
	struct line_param end_line;

	/* Background color for the area corresponding to the interval */
	struct interval_param interval;

	/* Number of pixels that the horizontal position at entity
	 * identification might be off wrt. the real start / end position of an
	 * interval */
	double entity_identification_fuzzy_px;
};

static const struct selection_layer_params SELECTION_LAYER_DEFAULT_PARAMS = {
	.start_line = {
		.color = AM_RGBA255(0xFF, 0xFF, 0xFF, 0x60),
		.width = 1.0
	},

	.end_line = {
		.color = AM_RGBA255(0xFF, 0xFF, 0xFF, 0x60),
		.width = 1.0
	},

	.interval = {
		.color = AM_RGBA255(0xFF, 0xFF, 0xFF, 0x40)
	},

	.entity_identification_fuzzy_px = 5
};

struct am_timeline_selection_layer {
	struct am_timeline_render_layer super;
	struct selection_layer_params params;
	struct am_timeline_selection_array selections;
};

static void render(struct am_timeline_selection_layer* sl, cairo_t* cr)
{
	struct am_timeline_renderer* r = sl->super.renderer;
	struct am_timeline_selection* it;
	double x1;
	double x2;
	double screen_top;
	double screen_bottom;

	screen_top = r->rects.lanes.y;
	screen_bottom = r->rects.lanes.y + r->rects.lanes.height;

	cairo_rectangle(cr, AM_RECT_ARGS(r->rects.lanes));
	cairo_clip(cr);

	am_timeline_selection_array_for_each_overlapping(sl, &r->visible_interval, it) {
		x1 = am_timeline_renderer_timestamp_to_x(r, it->interval.start);
		x2 = am_timeline_renderer_timestamp_to_x(r, it->interval.end);

		/* Rectangle for interval */
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(sl->params.interval.color));
		cairo_rectangle(cr, x1, screen_top, x2-x1, screen_bottom);
		cairo_fill(cr);

		/* vertical start line */
		cairo_set_line_width(cr, sl->params.start_line.width);
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(sl->params.start_line.color));

		cairo_move_to(cr, x1, screen_top);
		cairo_line_to(cr, x1, screen_bottom);
		cairo_stroke(cr);

		/* vertical end line */
		cairo_set_line_width(cr, sl->params.end_line.width);
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(sl->params.end_line.color));

		cairo_move_to(cr, x2, screen_top);
		cairo_line_to(cr, x2, screen_bottom);
		cairo_stroke(cr);
	}

	cairo_reset_clip(cr);
}

static struct am_timeline_render_layer*
instantiate(struct am_timeline_render_layer_type* t)
{
	struct am_timeline_selection_layer* l;

	if(!(l = malloc(sizeof(*l))))
		return NULL;

	am_timeline_render_layer_init(&l->super, t);

	am_timeline_selection_array_init(&l->selections);

	l->params = SELECTION_LAYER_DEFAULT_PARAMS;

	return (struct am_timeline_render_layer*)l;
}

static void destroy(struct am_timeline_selection_layer* sl)
{
	am_timeline_selection_array_destroy(&sl->selections);
}

/* Allocates, initializes and adds a selection entity to the list of entities
 * lst.
 *
 * Returns 0 on success, otherwise 1.
 */
static int add_selection_entity(
	struct am_timeline_selection_layer* sl,
	struct list_head* lst,
	enum am_timeline_selection_layer_selection_part type,
	const struct am_timeline_selection* selection)
{
	struct am_timeline_selection_layer_entity* e;

	if(!(e = malloc(sizeof(*e))))
		return 1;

	am_timeline_entity_init(&e->super, &sl->super,
				AM_TIMELINE_SELECTION_LAYER_SELECTION);

	e->type = type;
	e->selection = selection;

	am_timeline_entity_append(&e->super, lst);

	return 0;
}

static int identify_entities(struct am_timeline_selection_layer* sl,
			     struct list_head* lst,
			     double x, double y)
{
	struct am_timeline_renderer* r = sl->super.renderer;
	struct am_timeline_selection* it;
	struct am_interval qi;
	struct am_point p = { .x = x, .y = y };
	double fuzzy = sl->params.entity_identification_fuzzy_px;
	am_timestamp_t t;

	/* Only match selections visible in lane rect */
	if(!am_point_in_rect(&p, &r->rects.lanes))
		return 0;

	am_timeline_renderer_x_to_timestamp(r, x - fuzzy, &qi.start);
	am_timeline_renderer_x_to_timestamp(r, x + fuzzy, &qi.end);
	am_timeline_renderer_x_to_timestamp(r, x, &t);

	am_timeline_selection_array_for_each_overlapping(sl, &qi, it) {
		/* Position is on start of an interval */
		if(am_interval_contains_p(&qi, it->interval.start)) {
			if(add_selection_entity(
				   sl, lst,
				   AM_TIMELINE_SELECTION_LAYER_SELECTION_START,
				   it))
			{
				return 1;
			}
		}

		/* Position is on middle part */
		if(am_interval_contains_p(&it->interval, t)) {
			if(add_selection_entity(
				   sl, lst,
				   AM_TIMELINE_SELECTION_LAYER_SELECTION_INTERVAL,
				   it))
			{
				return 1;
			}
		}

		/* Position is on end of an interval */
		if(am_interval_contains_p(&qi, it->interval.end)) {
			if(add_selection_entity(
				   sl, lst,
				   AM_TIMELINE_SELECTION_LAYER_SELECTION_END,
				   it))
			{
				return 1;
			}
		}
	}

	return 0;
}

static void destroy_entity(struct am_timeline_selection_layer* sl,
			   struct am_timeline_entity* e)
{
	am_timeline_entity_destroy(e);
	free(e);
}

struct am_timeline_render_layer_type*
am_timeline_selection_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	if(!(t = malloc(sizeof(*t))))
		goto out_err;

	if(am_timeline_render_layer_type_init(t, "selection"))
		goto out_err_free;

	t->render = AM_TIMELINE_RENDER_LAYER_RENDER_FUN(render);
	t->instantiate = AM_TIMELINE_RENDER_LAYER_INSTANTIATE_FUN(instantiate);
	t->destroy = AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(destroy);
	t->identify_entities = AM_TIMELINE_RENDER_LAYER_IDENTIFY_ENTITIES_FUN(identify_entities);
	t->destroy_entity = AM_TIMELINE_RENDER_LAYER_DESTROY_ENTITY_FUN(destroy_entity);

	return t;

out_err_free:
	free(t);
out_err:
	return NULL;
}

/* Adds a new selection with the interval equal to *i.
 *
 * Returns a pointer to the newly created selection object or NULL on failure.
 */
const struct am_timeline_selection* am_timeline_selection_layer_add_selection(
	struct am_timeline_selection_layer* sl,
	const struct am_interval* i)
{
	struct am_timeline_selection* sel;

	if(!(sel = am_timeline_selection_array_reserve_sorted(&sl->selections,
							      i->end)))
	{
		return NULL;
	}

	sel->interval = *i;

	return sel;
}

/* Updates the interval of the selection s to *new_interval. WARNING: If
 * successful, this invalidates all pointers to selections obtained from
 * previous function calls, including the pointer passed in s.
 *
 * Returns a pointer to the updated selection or NULL on failure.
 */
const struct am_timeline_selection* am_timeline_selection_layer_update_selection(
	struct am_timeline_selection_layer* sl,
	const struct am_timeline_selection* s,
	const struct am_interval* new_i)
{
	struct am_timeline_selection* s_mut;
	size_t old_idx;
	size_t new_idx;

	if(!am_timeline_selection_array_is_element_ptr(
		   &sl->selections,
		   (struct am_timeline_selection*)s))
	{
		return NULL;
	}

	old_idx = am_timeline_selection_array_index(&sl->selections, s);
	s_mut = &sl->selections.elements[old_idx];

	/* Just updating the end -> Order in sl->selections does not change */
	if(s->interval.start == new_i->start) {
		s_mut->interval.end = new_i->end;
		return s;
	}

	/* Start moved, but last element -> Order doesn't change */
	if(new_i->start > s->interval.start &&
	   old_idx == sl->selections.num_elements-1)
	{
		s_mut->interval.start = new_i->start;
		s_mut->interval.end = new_i->end;
		return s;
	}

	new_idx = am_timeline_selection_array_insertpos(&sl->selections,
							new_i->start);

	if(new_idx > old_idx) {
		/* Move elements left */
		memmove(&sl->selections.elements[old_idx],
			&sl->selections.elements[new_idx-1],
			new_idx - old_idx - 1);

		sl->selections.elements[new_idx-1].interval = *new_i;
		return &sl->selections.elements[new_idx-1];
	} else if(new_idx <= old_idx) {
		if(new_idx < old_idx) {
			/* Move elements right */
			memmove(&sl->selections.elements[new_idx+1],
				&sl->selections.elements[new_idx],
				new_idx - old_idx - 1);
		}

		sl->selections.elements[new_idx].interval = *new_i;
		return &sl->selections.elements[new_idx];
	}

	/* Cannot happen */
	return NULL;
}

/* Deletes the selection s from the list of selections. WARNING: This
 * invalidates all pointers to selections obtained from previous function calls.
 *
 * Returns 0 on success, otherwise 1 (e.g., if s does not belong to the list of
 * selections).
 */
int am_timeline_selection_layer_delete_selection(
	struct am_timeline_selection_layer* sl,
	const struct am_timeline_selection* s)
{
	struct am_timeline_selection* snc = (struct am_timeline_selection*)s;

	if(!am_timeline_selection_array_is_element_ptr(&sl->selections, snc))
		return 1;

	am_timeline_selection_array_removep(&sl->selections, snc);

	return 0;
}

/* Returns the number of selections on sl */
size_t am_timeline_selection_layer_get_num_selections(
	const struct am_timeline_selection_layer* sl)
{
	return sl->selections.num_elements;
}

/* Returns the array with all selections of sl */
const struct am_timeline_selection*
am_timeline_selection_layer_get_selections(
	const struct am_timeline_selection_layer* sl)
{
	return sl->selections.elements;
}
