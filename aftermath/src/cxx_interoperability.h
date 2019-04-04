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

/* Defines a forward declaration for a class "name" if compiled with a C++
 * compiler. Otherwise it generates a forward declaration for a struct "name".
 */
#ifdef __cplusplus
#define AM_CXX_C_FWDDECL_CLASS_STRUCT(klass) class klass
#else
#define AM_CXX_C_FWDDECL_CLASS_STRUCT(strct) struct strct
#endif

/* Declares a field as a pointer to a class if compiled with a C++
 * compiler. Otherwise, it declares the field as a pointer to a struct.
 */
#ifdef __cplusplus
#define AM_CXX_C_DECL_CLASS_STRUCT_PTR_FIELD(klass, field_name) \
	klass* field_name
#else
#define AM_CXX_C_DECL_CLASS_STRUCT_PTR_FIELD(strct, field_name) \
	struct strct* field_name
#endif
