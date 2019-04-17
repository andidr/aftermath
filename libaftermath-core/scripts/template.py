#!/usr/bin/env python

# Author: Andi Drebes <andi@drebesium.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
# USA.

import argparse
import sys
import jinja2
import os
import re

class EnvironmentRelativePaths(jinja2.Environment):
    """Allows for the inclusion of files using a relative path specification."""
    def join_path(self, template, parent):
        return os.path.join(os.path.dirname(parent), template)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Render a template.')
    parser.add_argument('-t', '--template', metavar='T', type=argparse.FileType('r'),
                        help='Jinja2 template file', required=True)
    parser.add_argument('-m', '--module', metavar='D', type=str,
                        action="append",
                        help='Python file with definitions for the template')
    parser.add_argument('-o', '--output-file', type=argparse.FileType('w'),
                        help='Output file to which the tempate will be rendered')
    args = parser.parse_args()

    for m in args.module:
        if not os.path.exists(m):
            raise Exception("Could not find file '"+m+"'")

        if not (len(m) >= 3 and m[-3] == "." and m[-2] == "p" and m[-1] == "y"):
            raise Exception("File '"+m+"' does not have a .py extension")

    if args.output_file:
        outfile = args.output_file
    else:
        outfile = sys.stdout

    try:
        tpl_dir = os.path.dirname(args.template.name)
        tpl_basename = os.path.basename(args.template.name)

        env = EnvironmentRelativePaths(loader = jinja2.FileSystemLoader(tpl_dir),
                                       undefined=jinja2.StrictUndefined)

        defs = {"re" : re}
        for m in args.module:
            m_dir = os.path.dirname(m)
            m_name = os.path.basename(m)[:-3]
            sys.path.append(m_dir)
            t = __import__(m_name)

            if hasattr(t, "definitions"):
                defs.update(t.definitions())

            if hasattr(t, "filters"):
                env.filters.update(t.filters())

        template = env.get_template(tpl_basename)
        content = template.render(**defs)
        outfile.write(content)
    except:
        outfile.close()
        os.unlink(outfile.name)
        raise
