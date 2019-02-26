/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AM_GUIFACTORY_H
#define AM_GUIFACTORY_H

#include "../../Exception.h"
#include "../AftermathGUI.h"

#include <string>
#include <set>
#include <list>

#include <QWidget>
#include <QHBoxLayout>

extern "C" {
	#include <aftermath/core/object_notation.h>
}

/* Base class for objects that are able to create a widget from an object
 * notation group. */
class WidgetCreator {
	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg) { };
		};

		WidgetCreator(const std::string& groupName);
		virtual ~WidgetCreator() = default;

		/* Returns the widget type the creator's associated group
		 * name */
		const std::string& getGroupName() const {
			return this->groupName;
		}

		/* Instantiate the associated widget from a description in
		 * object notation */
		virtual QWidget*
		instantiate(const struct am_object_notation_node_group* n) = 0;

		/* Instantiates the associated widget with default values */
		virtual QWidget* instantiateDefault() = 0;

		/* Add the children to an earlier created widget. Will only be
		 * called by a GUI factory if isContainer() returns true and if
		 * the widget has actually been created by the creator.
		 */
		virtual void addChildren(
			const struct am_object_notation_node_group* n,
			QWidget* parent,
			std::list<QWidget*>& children) = 0;

		/* Returns true if a created widget can have child widgets */
		virtual bool isContainer() = 0;

	protected:
		const std::string groupName;
};

/**
 * Base class for widget creators creating container widgets.
 */
class ContainerWidgetCreator : public WidgetCreator {
	public:
		ContainerWidgetCreator(const std::string& groupName) :
			WidgetCreator(groupName)
		{ }

		virtual ~ContainerWidgetCreator() = default;

		bool isContainer()
		{
			return true;
		}
};

/**
 * Base class for container widget creators for which adding children is equal
 * to adding all children to the widget's layout.
 */
class LayoutContainerWidgetCreator : public ContainerWidgetCreator {
	public:
		LayoutContainerWidgetCreator(const std::string& groupName) :
			ContainerWidgetCreator(groupName)
		{ }

		virtual ~LayoutContainerWidgetCreator() = default;

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children)
		{
			for(auto child: children)
				parent->layout()->addWidget(child);
		}
};

/**
 * Base class for widget creators creating non-container widgets.
 */
class NonContainerWidgetCreator : public WidgetCreator {
	public:
		NonContainerWidgetCreator(const std::string& groupName) :
			WidgetCreator(groupName)
		{ }

		virtual ~NonContainerWidgetCreator() = default;

		bool isContainer()
		{
			return false;
		}

		void addChildren(const struct am_object_notation_node_group* n,
				 QWidget* parent,
				 std::list<QWidget*>& children)
		{
			throw WidgetCreator::Exception(
				"Trying to add children to a non-container "
				"widget.");
		}
};

/**
 * Comparison functor implementing < on WidgetCreators and strings
 */
class WidgetCreatorComparator {
	public:
		using is_transparent = void;

		bool operator()(const WidgetCreator* lhs,
				const WidgetCreator* rhs) const
		{
			return lhs->getGroupName() < rhs->getGroupName();
		}

		bool operator()(const std::string& t,
				const WidgetCreator* rhs) const
		{
			return t < rhs->getGroupName();
		}

		bool operator()(const WidgetCreator* rhs,
				const std::string& t) const
		{
			return rhs->getGroupName() < t;
		}
};

/**
 * Creator for the root widget. Expected format:
 *
 *   am_gui {
 *      @optional children: [ ... ],
 *      @optional id: "..."
 *   }
 */
class RootWidgetCreator : public LayoutContainerWidgetCreator {
	public:
		RootWidgetCreator() :
			LayoutContainerWidgetCreator("am_gui")
			{ }

		QWidget* instantiateDefault() {
			QWidget* w = new QWidget();

			w->setSizePolicy(QSizePolicy::Expanding,
					 QSizePolicy::Expanding);
			w->setLayout(new QVBoxLayout());
			w->layout()->setSpacing(0);

			return w;
		}

		QWidget* instantiate(
			const struct am_object_notation_node_group* n)
		{
			return this->instantiateDefault();
		}
};

/**
 * Creates widgets from object notations. The GUIFactory acts as a proxy that
 * selects the right widget creator for the widget type specified in object
 * notation.
 */
class GUIFactory {
	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg)
				{ }
		};

		class NoCreatorFoundException : public Exception {
			public:
				NoCreatorFoundException(const char* groupName) :
					Exception(
						std::string(
							"Could not find creator "
							"for group type ") +
						groupName +
						".")
				{ }
		};

		class ObjectNotationFileException : public Exception {
			public:
				ObjectNotationFileException(
					const std::string& message) :
					Exception(message)
				{ }
		};

		GUIFactory();
		~GUIFactory();

		void addCreator(WidgetCreator* c);
		WidgetCreator* findCreator(const std::string& groupName);

		void buildGUI(AftermathGUI* gui,
			      struct am_object_notation_node* root);

		void buildGUI(AftermathGUI* gui, const char* filename);

		std::string generateID(const std::string& prefix = "");

		const std::set<WidgetCreator*, WidgetCreatorComparator>&
		getWidgetCreators() const;

	protected:
		/**
		 * Collects information a widget that has been created, but not
		 * yet been added to a GUI.
		 */
		class WidgetMetaInfo {
			public:
				WidgetMetaInfo(QWidget* w, const std::string& id)
					: id(id), widget(w)
				{ }

				const std::string& getID() {
					return this->id;
				}

				QWidget* getWidget() {
					return this->widget;
				}

				void setWidget(QWidget* w) {
					this->widget = w;
				}

			protected:
				std::string id;
				QWidget* widget;
		};

		std::set<WidgetCreator*, WidgetCreatorComparator> creators;
		RootWidgetCreator rootCreator;
		uint64_t currID;

		void commitWidgets(AftermathGUI& gui,
				   std::list<WidgetMetaInfo>& widgets);

		QWidget* buildWidget(
			std::list<WidgetMetaInfo>& widgets,
			const struct am_object_notation_node_group* n);

		void buildChildren(
			std::list<WidgetMetaInfo>& widgets,
			WidgetCreator* creator,
			const struct am_object_notation_node_group* n,
			QWidget* parent,
			struct am_object_notation_node* children);
};

#endif
