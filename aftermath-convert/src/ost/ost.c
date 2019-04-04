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

#include "ost.h"
#include "v16.h"
#include <aftermath/core/on_disk.h>

/* Converts a file in an old OST format to the most recent OST format. The input
 * file handle must be positioned after the magic number.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_convert_trace_ost(
	FILE* fp_in, FILE* fp_out, struct am_io_error_stack* estack)
{
	uint32_t version;

	if(fread(&version, sizeof(version), 1, fp_in) != 1) {
		am_io_error_stack_push(estack,
				       AM_IOERR_READ,
				       "Could not read OST version.");
		return 1;
	}

	version = am_int32_letoh(version);

	switch(version) {
		case 13:
		case 14:
		case 15:
		case 16:
			return v16_convert_trace(fp_in, fp_out, estack);
		default:
			am_io_error_stack_push(estack,
					       AM_IOERR_READ,
					       "Version %" PRIu32 " is not "
					       "supported.",
					       version);
			break;
	}

	return 0;
}
