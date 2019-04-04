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

#ifndef AM_DFG_QT_PROCESSOR_H
#define AM_DFG_QT_PROCESSOR_H

#include <QObject>

extern "C" {
	#include <aftermath/core/dfg_graph.h>
	#include <aftermath/core/dfg_schedule.h>
}

/**
 * Proxy that can be associated with the processDFGNodeSignal of a widget with
 * an associated DFG node and that triggers evalation of a DFG graph upon
 * reception of the signal.
 */
class DFGQTProcessor : public QObject {
	Q_OBJECT

	public:
		DFGQTProcessor();

		void setDFG(struct am_dfg_graph* g) noexcept;
		struct am_dfg_graph* getDFG() noexcept;

		void enable();
		void disable();
		void setEnabled(bool b);
		bool isEnabled();

	public slots:
		void DFGNodeTriggered(struct am_dfg_node* n);

	protected:
		struct am_dfg_graph* dfgGraph;
		bool enabled;
};

#endif
