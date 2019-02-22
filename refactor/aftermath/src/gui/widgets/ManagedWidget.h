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

#ifndef AM_MANAGEDWIDGET_H
#define AM_MANAGEDWIDGET_H

#include "../../cxx_extras.h"
#include <limits>
#include <cstdlib>
#include <QWidget>

/* Mixin class to tag widgets that are proper to Aftermath */
class ManagedWidget {
	public:
		virtual ~ManagedWidget() { }

		/* Returns the widget type name as a string constant */
		virtual const char* getName() = 0;

		/* Detaches the widget from its parent */
		virtual void unparent() = 0;

		/* Returns the QObject associated to the widget */
		virtual QObject* getQObject() = 0;
};

/* Declares a class that wraps the type QTTYPE with a ManagedWidget */
#define AM_ALIAS_WIDGET(AMTYPE, QTTYPE, NAME)			\
	class AMTYPE : public QTTYPE, public ManagedWidget	\
	{							\
		public:					\
			virtual const char* getName() {	\
				return NAME;			\
			}					\
								\
			virtual void unparent() {		\
				this->setParent(NULL);		\
			}					\
								\
			virtual QObject* getQObject() {	\
				return this;			\
			}					\
	};


/* Class to tag managed widgets that can have children */
class ManagedContainerWidget : public ManagedWidget {
	public:
		/* Returns the number of children */
		virtual size_t getNumChildren() = 0;

		/* Returns the maximum number of children that the container can
		 * accept */
		virtual size_t getMaxChildren() {
			return std::numeric_limits<size_t>::max();
		}

		/* Returns the n-th child from the container */
		virtual QObject* getNthChild(size_t n) = 0;

		/* Adds a child at the zero-based index idx, such that all
		 * widgets following (including the one currently at index idx)
		 * are shifted */
		virtual void addChild(QWidget* w, size_t idx) = 0;

		/* Remove the widget w from the list of children */
		virtual void removeChild(QWidget* w) = 0;
};

/* Class to tag managed container widgets that can only have fixed number of
 * children */
template<size_t nmax_children>
class ManagedRestrictedContainerWidget : public ManagedContainerWidget {
	public:
		virtual size_t getMaxChildren() {
			return nmax_children;
		}
};

/* Declares a managed container widget class wrapping the type QTTYPE with a
 * restricted number of children MAX_CHILDREN. QTTYPE must use it's layout to
 * organize its children. The type parameter must be class that implements the
 * method strconst() returning the widget type's name.
 */
#define AM_RESTRICTED_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET(AMTYPE, QTTYPE, \
								LAYOUTTYPE,	\
								MAX_CHILDREN)	\
	template<typename NAME_T>						\
	class AMTYPE : public ManagedRestrictedContainerWidget<MAX_CHILDREN>,	\
		       public QTTYPE						\
	{									\
		public:							\
			virtual void unparent() {				\
				this->setParent(NULL);				\
			}							\
										\
			virtual size_t getNumChildren() {			\
				return this->layout()->count();		\
			}							\
										\
			virtual void addChild(QWidget* w, size_t idx) {	\
				LAYOUTTYPE* l;					\
				l = static_cast<LAYOUTTYPE*>(this->layout());	\
										\
				if(this->getNumChildren() <= idx)		\
					l->addWidget(w);			\
				else						\
					l->insertWidget(idx, w);		\
										\
				w->setVisible(true);				\
			}							\
										\
			virtual void removeChild(QWidget* w) {			\
				this->layout()->removeWidget(w);		\
			}							\
										\
			virtual const char* getName() {			\
				return NAME_T::strconst();			\
			}							\
										\
			virtual QObject* getQObject() {			\
				return this;					\
			}							\
										\
			virtual QObject* getNthChild(size_t n) {		\
				QLayoutItem* li = this->layout()->itemAt(n);	\
										\
				return li->widget();				\
			}							\
	};

/* Same as AM_RESTRICTED_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET, but with an
 * arbitrary number of children */
#define AM_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET(AMTYPE, QTTYPE, LAYOUTTYPE)\
	AM_RESTRICTED_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET(		\
		AMTYPE, QTTYPE, LAYOUTTYPE, std::numeric_limits<size_t>::max())

/* Same as AM_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET, declares a class for
 * the string constant and uses it as the type parameter */
#define AM_LAYOUTED_ALIAS_CONTAINER_WIDGET(AMTYPE, QTTYPE, LAYOUTTYPE, NAME)	\
	AM_CXX_MKSTRCONSTTYPE(AMTYPE##StrType, NAME)				\
	AM_TEMPLATED_LAYOUTED_ALIAS_CONTAINER_WIDGET(				\
		AMTYPE##Templated, QTTYPE, LAYOUTTYPE)				\
	using AMTYPE = AMTYPE##Templated<AMTYPE##StrType>;

/* Declares a managed container widget class wrapping the type QTTYPE with a
 * restricted number of children MAX_CHILDREN. QTTYPE must organize its children
 * directly without a layout. The type parameter must be class that implements
 * the method strconst() returning the widget type's name.
 */
#define AM_RESTRICTED_TEMPLATED_ALIAS_CONTAINER_WIDGET(			\
	AMTYPE, QTTYPE, MAX_CHILDREN)						\
	template<typename NAME_T>						\
	class AMTYPE : public ManagedRestrictedContainerWidget<MAX_CHILDREN>,	\
		       public QTTYPE						\
	{									\
		public:							\
			virtual void unparent() {				\
				this->setParent(NULL);				\
			}							\
										\
			virtual size_t getNumChildren() {			\
				return this->count();				\
			}							\
										\
			virtual void addChild(QWidget* w, size_t idx) {	\
				if(this->getNumChildren() <= idx)		\
					this->addWidget(w);			\
				else						\
					this->insertWidget(idx, w);		\
										\
				w->setVisible(true);				\
			}							\
										\
			virtual void removeChild(QWidget* w) {			\
				w->setParent(NULL);				\
			}							\
										\
			virtual const char* getName() {			\
				return NAME_T::strconst();			\
			}							\
										\
			virtual QObject* getQObject() {			\
				return this;					\
			}							\
	};

/* Same as AM_RESTRICTED_TEMPLATED_ALIAS_CONTAINER_WIDGET, but with an arbitrary
 * number of children */
#define AM_TEMPLATED_ALIAS_CONTAINER_WIDGET(AMTYPE, QTTYPE)		\
	AM_RESTRICTED_TEMPLATED_ALIAS_CONTAINER_WIDGET(		\
		AMTYPE, QTTYPE, std::numeric_limits<size_t>::max())

/* Same as AM_TEMPLATED_ALIAS_CONTAINER_WIDGET, declares a class for the string
 * constant and uses it as the type parameter */
#define AM_ALIAS_CONTAINER_WIDGET(AMTYPE, QTTYPE, NAME)		\
	AM_CXX_MKSTRCONSTTYPE(AMTYPE##StrType, NAME)			\
	AM_TEMPLATED_ALIAS_CONTAINER_WIDGET(AMTYPE##Templated, QTTYPE)	\
	using AMTYPE = AMTYPE##Templated<AMTYPE##StrType>;

#endif
