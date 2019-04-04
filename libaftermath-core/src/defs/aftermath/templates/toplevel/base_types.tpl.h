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

#ifndef AM_BASE_TYPES_H
#define AM_BASE_TYPES_H

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

{% for t in aftermath.config.getBaseTypes() -%}
{%- if t.getComment() %}
/* {{t.getComment()}} */
{%- endif %}
typedef {{t.getAliasedType().getCType()}} {{t.getName()}};

{%- if isinstance(t, aftermath.types.FixedWidthIntegerType) %}
#define {{t.getName()|upper}}_FMT {{t.getAliasedType().getFormatStringSym()}}
#define {{t.getName()|upper}}_BITS {{t.getNumBits()}}
#define {{t.getName()|upper}}_MIN {{t.getMinValueSym()}}
#define {{t.getName()|upper}}_MAX {{t.getMaxValueSym()}}
#define {{t.getName()|upper}}_SIGNED {{t.getSignedSym()}}
#define {{t.getName()|upper}}_MAX_DECIMAL_DIGITS {{t.getMaxDecimalDigits()}}
{# #}
{%- endif -%}
{% endfor -%}

{# #}
static inline void am_free_charp(char** pstr)
{
	free(*pstr);
}

#endif
