/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * ************************************************************************
 * * THIS FILE IS PART OF THE CODE RELEASED UNDER THE LGPL, VERSION 2.1   *
 * * UNLIKE THE MAJORITY OF THE CODE OF LIBAFTERMATH-CORE, RELEASED UNDER *
 * * THE GPL, VERSION 2.                                                  *
 * ************************************************************************
 *
 * This file can be redistributed it and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_ON_DISK_DEFAULT_TYPE_IDS_H
#define AM_ON_DISK_DEFAULT_TYPE_IDS_H

#include <inttypes.h>

{% set dsk_types = aftermath.config.getDskTypes() -%}

/* Default type IDs for on-disk frames; Only to be used when writing traces,
 * since the mapping from IDs to types can vary arbitrarily between traces.
 */
struct am_default_on_disk_type_ids {
{% for t in dsk_types.filterByTag(aftermath.tags.dsk.Frame) -%}
{# #}	uint32_t {{t.getName()}};
{% endfor -%}
};

extern struct am_default_on_disk_type_ids am_default_on_disk_type_ids;

#endif
