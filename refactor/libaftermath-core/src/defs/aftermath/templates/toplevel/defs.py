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

import sys
import os
import pkgutil

# Main definition file used when processing top-level templates

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

import aftermath

def import_rec(package):
    """Imports all submodules of a module recursively"""

    if not hasattr(package, "__path__"):
        return

    for importer, modname, ispkg in pkgutil.iter_modules(package.__path__, package.__name__+"."):
        module = __import__(modname, fromlist = "foo")
        import_rec(module)

# Import everything from the aftermath module recursively in order to make all
# submodules available in templates without having to import them and add to the
# template environment individually
import_rec(aftermath)

aftermath.config.finalize()

def definitions():
    return {
        "isinstance" : isinstance,
        "aftermath" : aftermath
    }
