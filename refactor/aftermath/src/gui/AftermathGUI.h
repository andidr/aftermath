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

#ifndef AFTERMATHGUI_H
#define AFTERMATHGUI_H

#include "Exception.h"
#include <map>
#include <string>
#include <set>

class QWidget;

/**
 * Collection of all Widgets in an instance of Aftermath
 */
class AftermathGUI {
	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg)
				{ }
		};

		class DuplicateIDException : public Exception {
			public:
				DuplicateIDException(const std::string& id) :
					Exception(std::string("Id "+id+
							      " already "
							      "exists."))
				{ }
		};

		class NonexistentIDException : public Exception {
			public:
				NonexistentIDException(const std::string& id) :
					Exception(std::string("Id "+id+
							      " does not "
							      "exist."))
				{ }
		};

		class NonexistentWidgetException : public Exception {
			public:
				NonexistentWidgetException() :
					Exception(std::string("Widget does not "
							      "have an ID"))
				{ }
		};

		AftermathGUI();
		~AftermathGUI();

		void addWidget(QWidget* w, const std::string& id);

		void removeWidget(const std::string& id);
		void removeWidget(QWidget* w);
		void removeWidgetCheckDestroy(QWidget* w);
		QWidget* getWidget(const std::string& id);
		const std::string& getID(QWidget* w);

		QWidget* getRootWidget() noexcept;
		void setRootWidget(QWidget* w);
		void setRootWidgetNE(QWidget* w) noexcept;

		void markWidgetBound(QWidget* w) noexcept;
		void markWidgetUnbound(QWidget* w);
		bool isWidgetUnbound(QWidget* w) noexcept;

		/* Calls the functor f for all widgets of type T with a pointer
		 * to the widget */
		template<typename T, typename F> void applyToWidgetsOfType(F f) {
			for(auto w: this->widgets) {
				T* wcast = dynamic_cast<T*>(w.second);

				if(wcast)
					f(wcast);
			}
		}

	protected:
		std::map<const std::string, QWidget*> widgets;
		std::map<QWidget*, const std::string> revWidgets;
		std::set<QWidget*> unboundWidgets;
		QWidget* root;
};

#endif
