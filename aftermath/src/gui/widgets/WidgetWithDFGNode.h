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

#ifndef AM_WIDGETWITHDFGNODE_H
#define AM_WIDGETWITHDFGNODE_H

#include <QMetaType>

extern "C" {
	#include <aftermath/core/dfg_node.h>
}

/* This macro adds a a few definitions to a class:
 *
 *   - A struct am_dfg_node* dfgNode
 *   - A setter / getter method for the node (setDFGNode and getDFGNode)
 *   - A signal processDFGNodeSignal(struct am_dfg_node*)
 *   - A function processDFGNode() that emits the signal
 *
 * The purpose of this macro is to simplify the association of a widget with a
 * DFG node. A widget that wants to make use of this needs to invoke the macro
 * at the very beginning of the class declaration, before Q_OBJECT, e.g.,
 *
 *   class FooWidget : public QWidget {
 *     AM_WIDGETWITHDFGNODE_DECLS
 *     Q_OBJECT
 *     ...
 *   }
 *
 * This can't be implemented in a class (e.g., WidgetWithDFGNode) that a
 * subtyped QT widget inherits from, since this would lead to a diamond-like
 * inheritance relationship:
 *
 *   - The WidgetWithDFGNode class would have to use Q_OBJECT to declare its
 *     signals and thus inherit from QObject
 *
 *   - The subtyped QT widget inherits from QWidget, which in turn inherits from
 *     QObject
 */
#define AM_WIDGETWITHDFGNODE_DECLS						\
	public:								\
		/**								\
		 * Associates a DFG node with the widgets			\
		 */								\
		void setDFGNode(struct am_dfg_node* n) noexcept		\
		{ this->dfgNode = n; }						\
										\
		/**								\
		 * Returns the currently associated DFG node of the widget	\
		 */								\
		struct am_dfg_node* getDFGNode() noexcept 			\
		{ return this->dfgNode; }					\
										\
		Q_SIGNAL void processDFGNodeSignal(struct am_dfg_node* n);	\
										\
		/* Indicate that the class has a DFG node */			\
		Q_PROPERTY(struct am_dfg_node* DFGNode READ getDFGNode)	\
										\
	protected:								\
		/**								\
		 * Triggers processing of the node. If the node is actually	\
		 * processed depends on whether the processDFGNodeSignal of	\
		 * the widget is connected with an appropriate processor.	\
		 */								\
		void processDFGNode()						\
		{ if(this->dfgNode) emit processDFGNodeSignal(this->dfgNode); } \
										\
		struct am_dfg_node* dfgNode = NULL;

Q_DECLARE_METATYPE(struct am_dfg_node*)
Q_DECLARE_OPAQUE_POINTER(struct am_dfg_node*)

/* Declares a wrapper class named TYPE for QTTYPE that adds functionality for
 * DFG node management */
#define AM_ALIAS_WIDGETWITHDFGNODE(TYPE, QTTYPE)\
	class TYPE : public QTTYPE {		\
		AM_WIDGETWITHDFGNODE_DECLS	\
		Q_OBJECT			\
						\
		public:			\
		TYPE(QWidget* parent = NULL) :	\
			QTTYPE(parent)		\
		{ }				\
	};

#endif
