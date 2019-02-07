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

#include "TelamonCandidateTreeWidget.h"
#include "../../dfg/nodes/gui/telamon_candidate_tree.h"
#include "../../Exception.h"

#include <QMouseEvent>
#include <iostream>

extern "C" {
	#include <aftermath/core/telamon.h>
}

TelamonCandidateTreeWidget::TelamonCandidateTreeWidget() :
	useIntervals(false), candidateUnderMouse(NULL)
{
	am_telamon_candidate_tree_renderer_init(&this->renderer);
	this->setMouseTracking(true);
	this->setFocusPolicy(Qt::ClickFocus);
}

TelamonCandidateTreeWidget::~TelamonCandidateTreeWidget()
{
	am_telamon_candidate_tree_renderer_destroy(&this->renderer);
}

void TelamonCandidateTreeWidget::resizeEvent(QResizeEvent *event)
{
	super::resizeEvent(event);

	am_telamon_candidate_tree_renderer_set_width(&this->renderer, width());
	am_telamon_candidate_tree_renderer_set_height(&this->renderer, height());
}

void TelamonCandidateTreeWidget::wheelEvent(QWheelEvent* event)
{
	struct am_point p = {
		(double)event->x(),
		(double)event->y()
	};

	if(event->delta() > 0)
		am_telamon_candidate_tree_renderer_zoom_in(&this->renderer, &p);
	else
		am_telamon_candidate_tree_renderer_zoom_out(&this->renderer, &p);

	this->update();
}

/**
 * Called when the mouse is moved in navigation mode
 */
void TelamonCandidateTreeWidget::mouseMoveNavigate(const struct am_point* screen_pos)
{
	double dx;
	double dy;
	double gdx;
	double gdy;
	double gx;
	double gy;

	dx = this->mouseStartPos.x - screen_pos->x;
	dy = this->mouseStartPos.y - screen_pos->y;

	gdx = am_telamon_candidate_tree_renderer_screen_w_to_graph(&this->renderer, dx);
	gdy = am_telamon_candidate_tree_renderer_screen_h_to_graph(&this->renderer, dy);

	gx = this->graphStartPos.x + gdx;
	gy = this->graphStartPos.y + gdy;

	am_telamon_candidate_tree_renderer_set_offset(&this->renderer, gx, gy);

	this->update();
}

void TelamonCandidateTreeWidget::mouseMoveEvent(QMouseEvent* event)
{
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->renderer.valid)
		return;

	if(this->mouseMode == MOUSE_MODE_NAVIGATE) {
		this->mouseMoveNavigate(&screen_pos);
	} else {
		this->setCursor(Qt::OpenHandCursor);
		this->updateCandidateUnderMouse(&screen_pos);
	}
}

/* Updates the internal reference to the candidate under the mouse cursor */
void TelamonCandidateTreeWidget::updateCandidateUnderMouse(
	const struct am_point* p)
{
	struct am_telamon_candidate* c;

	c = am_telamon_candidate_tree_renderer_candidate_at(
		&this->renderer, p);

	if(c != this->candidateUnderMouse) {
		this->candidateUnderMouse = c;
		this->checkTriggerHoverCandidatePort();
	}
}

/* Adds the first candidate found at position p (in screen coordinates) to the
 * list of selected candidates if the candidate has not already been
 * selected. Otherwise, the candidate is removed from the selection. */
void TelamonCandidateTreeWidget::toggleCandidateSelectionAt(const struct am_point* p)
{
	struct am_telamon_candidate* c;

	c = am_telamon_candidate_tree_renderer_candidate_at(
		&this->renderer, p);

	if(c) {
		auto it = this->selections.find(c);

		if(it != this->selections.end()) {
			this->selections.erase(it);
			am_telamon_candidate_tree_renderer_unselect(
				&this->renderer, c);
		} else {
			this->selections.insert(c);
			am_telamon_candidate_tree_renderer_select(
				&this->renderer, c);
		}

		this->update();
		this->checkTriggerSelectionPort();
	}
}

/* Unselects all previously selected nodes */
void TelamonCandidateTreeWidget::clearSelection()
{
	for(struct am_telamon_candidate* c: this->selections) {
		am_telamon_candidate_tree_renderer_unselect(
			&this->renderer, c);
	}

	this->selections.clear();
	this->checkTriggerSelectionPort();
	this->update();
}

/* Starts mouse navigation (mouse is at position p in screen coordinates) */
void TelamonCandidateTreeWidget::startNavigation(const struct am_point* p)
{
	struct am_point graph_pos;

	this->mouseStartPos = *p;

	am_telamon_candidate_tree_renderer_screen_to_graph(
		&this->renderer, p, &graph_pos);

	am_telamon_candidate_tree_renderer_get_offset(
		&this->renderer, &this->graphStartPos.x, &this->graphStartPos.y);

	this->setCursor(Qt::ClosedHandCursor);
	this->mouseMode = MOUSE_MODE_NAVIGATE;
}

void TelamonCandidateTreeWidget::mousePressEvent(QMouseEvent* event)
{
	struct am_point screen_pos = {
		.x = (double)event->x(),
		.y = (double)event->y()
	};

	if(!this->renderer.valid)
		return;

	/* Handle selections */
	if(event->buttons() == Qt::LeftButton) {
		if(event->modifiers() & Qt::ControlModifier)
			this->toggleCandidateSelectionAt(&screen_pos);
	} else if(event->buttons() == Qt::RightButton) {
		this->clearSelection();
	}

	/* Only react to left mouse button for navigation */
	if(event->button() == Qt::LeftButton &&
	   event->modifiers() == Qt::NoButton)
	{
		this->startNavigation(&screen_pos);
	}
}

void TelamonCandidateTreeWidget::mouseReleaseEvent(QMouseEvent* event)
{
	this->mouseMode = MOUSE_MODE_NONE;
	this->setCursor(Qt::OpenHandCursor);
	this->update();
}

/* Set the candidate tree to be rendered by the widget */
void TelamonCandidateTreeWidget::setCandidateTree(struct am_telamon_candidate* root)
{
	if(am_telamon_candidate_tree_renderer_set_root(&this->renderer, root))
		throw AftermathException("Could not set root");

	if(this->candidateUnderMouse) {
		this->candidateUnderMouse = NULL;
		this->checkTriggerHoverCandidatePort();
	}

	this->update();
}

void TelamonCandidateTreeWidget::cairoPaintEvent(cairo_t* cr)
{
	if(this->renderer.valid)
		am_telamon_candidate_tree_renderer_render(&this->renderer, cr);
}

/* Returns the set of selected nodes as a vector */
std::vector<struct am_telamon_candidate*> TelamonCandidateTreeWidget::getSelection()
{
	std::vector<struct am_telamon_candidate*> ret;

	for(struct am_telamon_candidate* c: this->selections)
		ret.push_back(c);

	return ret;
}

/* Invokes DFG node processing to notify about changed selections. */
void TelamonCandidateTreeWidget::checkTriggerSelectionPort()
{
	if(this->dfgNode) {
		am_dfg_port_mask_reset(&this->dfgNode->required_mask);
		this->dfgNode->required_mask.push_new =
			(1 << AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_SELECTIONS_OUT_PORT);
		this->processDFGNode();
	}
}

/* Invokes DFG node processing to notify about changed candidate under the mouse
 * cursor. */
void TelamonCandidateTreeWidget::checkTriggerHoverCandidatePort()
{
	if(this->dfgNode) {
		am_dfg_port_mask_reset(&this->dfgNode->required_mask);
		this->dfgNode->required_mask.push_new =
			(1 << AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_HOVER_CANDIDATE_OUT_PORT);
		this->processDFGNode();
	}
}

/* Limits rendering to candidates whose discovery was within at least one of the
 * intervals passed as an argument */
void TelamonCandidateTreeWidget::setIntervals(
	const struct am_interval* intervals,
	size_t num_intervals)
{
	try {
		this->resetIntervals();

		for(size_t i = 0; i < num_intervals; i++)
			this->intervals.push_back(intervals[i]);

		this->useIntervals = true;

		am_telamon_candidate_tree_renderer_set_intervals(
			&this->renderer,
			&this->intervals[0],
			num_intervals);
	} catch(...) {
		this->resetIntervals();
		throw;
	}

	this->update();
}

/* Resets the intervals for rendering, such that all candidates become
 * visible */
void TelamonCandidateTreeWidget::resetIntervals()
{
	this->intervals.clear();
	this->useIntervals = false;

	am_telamon_candidate_tree_renderer_reset_intervals(&this->renderer);
	this->update();
}

/* Returns the candidates that is under the mouse pointer. If multiple
 * candidates would match, the first one as determined by the renderer is
 * returned. If no candidate is under the mouse, the function returns NULL. */
struct am_telamon_candidate* TelamonCandidateTreeWidget::getCandidateUnderMouse()
{
	return this->candidateUnderMouse;
}

/* Modifies the selection by applying the functor f on each currently selected
 * candidate. The functor has the following prototype:

     f(struct am_telamon_candidate* c,
       std::set<struct am_telamon_candidate*>& newset)

   The functor may modify the set newset, e.g., add new elements. If keepOld is
   true, the elements in newset will be added to the current
   selection. Otherwise, the current selection is replaced with the final set.
*/
template<typename F> void
TelamonCandidateTreeWidget::modifySelection(F& f, bool keepOld)
{
	if(this->selections.size() == 0)
		return;

	std::set<struct am_telamon_candidate*> newSel;

	for(auto c: this->selections) {
		f(c, newSel);

		if(keepOld)
			newSel.insert(c);

		am_telamon_candidate_tree_renderer_unselect(&this->renderer, c);
	}

	for(auto c: newSel)
		am_telamon_candidate_tree_renderer_select(&this->renderer, c);

	this->selections.swap(newSel);
	this->update();
	this->checkTriggerSelectionPort();
}

/* Selects / adds all parents of the currently selected candidates */
void TelamonCandidateTreeWidget::selectParents(bool keepOld)
{
	auto f = [](struct am_telamon_candidate* c,
		    std::set<struct am_telamon_candidate*>& newset)
		{
			if(c->parent)
				newset.insert(c->parent);
		};

	this->modifySelection(f, keepOld);
}

/* Selects / adds the first child of each selected candidate */
void TelamonCandidateTreeWidget::selectFirstChildren(bool keepOld)
{
	auto f = [](struct am_telamon_candidate* c,
		   std::set<struct am_telamon_candidate*>& newset)
		{
			if(am_telamon_candidate_has_children(c))
				newset.insert(c->children[0]);
		};

	this->modifySelection(f, keepOld);
}

/* Selects / adds the previous sibling of each selected candidate */
void TelamonCandidateTreeWidget::selectPrevSiblings(bool keepOld)
{
	auto f = [](struct am_telamon_candidate* c,
		   std::set<struct am_telamon_candidate*>& newset)
		{
			struct am_telamon_candidate* sibling;

			if((sibling = am_telamon_candidate_get_prev_sibling(c)))
				newset.insert(sibling);
		};

	this->modifySelection(f, keepOld);
}

/* Selects / adds the next sibling of each selected candidate */
void TelamonCandidateTreeWidget::selectNextSiblings(bool keepOld)
{
	auto f = [](struct am_telamon_candidate* c,
		   std::set<struct am_telamon_candidate*>& newset)
		{
			struct am_telamon_candidate* sibling;

			if((sibling = am_telamon_candidate_get_next_sibling(c)))
				newset.insert(sibling);
		};

	this->modifySelection(f, keepOld);
}

void TelamonCandidateTreeWidget::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Up)
		this->selectParents(event->modifiers() & Qt::ShiftModifier);
	else if(event->key() == Qt::Key_Down)
		this->selectFirstChildren(event->modifiers() & Qt::ShiftModifier);
	else if(event->key() == Qt::Key_Left)
		this->selectPrevSiblings(event->modifiers() & Qt::ShiftModifier);
	else if(event->key() == Qt::Key_Right)
		this->selectNextSiblings(event->modifiers() & Qt::ShiftModifier);
	else
		super::keyPressEvent(event);
}
