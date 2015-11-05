/**
 * Copyright (C) 2014 Quentin Bunel <quentin.bunel@gmail.com>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "color.h"

double task_type_colors[NUM_TASK_TYPE_COLORS][3] = {{COL_NORM(102.0), COL_NORM(  0.0), COL_NORM(255.0)},
					       {COL_NORM(136.0), COL_NORM( 66.0), COL_NORM( 29.0)},
					       {COL_NORM(240.0), COL_NORM(195.0), COL_NORM(  0.0)},
					       {COL_NORM(253.0), COL_NORM(108.0), COL_NORM(158.0)},
					       {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(255.0)},
					       {COL_NORM(247.0), COL_NORM( 35.0), COL_NORM( 12.0)},
					       {COL_NORM( 22.0), COL_NORM(184.0), COL_NORM( 78.0)},
					       {COL_NORM(  9.0), COL_NORM(106.0), COL_NORM(  9.0)},
					       {COL_NORM(103.0), COL_NORM(113.0), COL_NORM(121.0)},
					       {COL_NORM( 37.0), COL_NORM(253.0), COL_NORM(233.0)}};

double node_colors_dbl[NUM_NODE_COLORS][3] = {{COL_NORM(0xFB), COL_NORM(0xB4), COL_NORM(0xAB)},
					      {COL_NORM(0xB3), COL_NORM(0xCD), COL_NORM(0xE3)},
					      {COL_NORM(0xCC), COL_NORM(0xEB), COL_NORM(0xC5)},
					      {COL_NORM(0xDE), COL_NORM(0xCB), COL_NORM(0xE4)},
					      {COL_NORM(0xFE), COL_NORM(0xD9), COL_NORM(0xA6)},
					      {COL_NORM(0xFF), COL_NORM(0xFF), COL_NORM(0xCC)},
					      {COL_NORM(0xE5), COL_NORM(0xD8), COL_NORM(0xBD)},
					      {COL_NORM(0xFD), COL_NORM(0xDA), COL_NORM(0xEC)}};

double state_colors[][3] = {{COL_NORM(117.0), COL_NORM(195.0), COL_NORM(255.0)},
			    {COL_NORM(  0.0), COL_NORM(  0.0), COL_NORM(255.0)},
			    {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(255.0)},
			    {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(  0.0)},
			    {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(174.0)},
			    {COL_NORM(179.0), COL_NORM(  0.0), COL_NORM(  0.0)},
			    {COL_NORM(  0.0), COL_NORM(255.0), COL_NORM(  0.0)},
			    {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(  0.0)},
			    {COL_NORM(235.0), COL_NORM(  0.0), COL_NORM(  0.0)}};
