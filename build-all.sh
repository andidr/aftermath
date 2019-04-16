#!/bin/bash

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

die() {
    echo "$1" >&2
    exit 1
}

do_configure() {
    SUBPROJECT="$1"
    shift
    ARGS=("$@")

    printf "%s" "Creating vpath directory for $SUBPROJECT... "
    mkdir -p "build/$SUBPROJECT" || die "failed."
    echo "done."
    echo "Configuring $SUBPROJECT..."
    SRCDIR="$PWD/$SUBPROJECT"
    ( cd "build/$SUBPROJECT" && "$SRCDIR/configure" "${ARGS[@]}" ) || die "Could not configure $SUBPROJECT"
}

do_make() {
    SUBPROJECT="$1"
    shift
    ARGS=("$@")

    echo "Building $SUBPROJECT..."
    ( cd "build/$SUBPROJECT" && make "${ARGS[@]}" && make install "${ARGS[@]}" ) || die "Could not build $SUBPROJECT"
}

print_help() {
    echo "Usage: $0 [options]"
    echo "Build all Aftermath subprojets and install to common location"
    echo
    echo "Options:"
    echo "  -h, --help               Show this help"
    echo "  --bootstrap              Force invocation of bootstrap script for each"
    echo "                           subproject"
    echo "  --clean                  Deletes files from previous builds before compiling"
    echo "  --debug                  Build debugging version"
    echo "  --enable-python          Enable Python bindings"
    echo "  --install-p              Invoke install with -p parameter to create symlinks"
    echo "                           from installed header files to header files from build"
    echo "                           directory to avoid rebuilding depending subprojects"
    echo "                           when unchanged header files are installed"
    echo "  -j N                     Builds with up to N jobs in parallel"
    echo "  -v, --verbose            Passes V=1 to make, causing full compilation commands"
    echo "                           to be shown"
    echo "  --local,--local-install  Creates a folder 'install' and installs all packages"
    echo "                           subprojects into it"
    echo "  --prefix=PATH            Installs all packages into PATH"

}

PREFIX="/usr/local"
JOBS=""
VERBOSITY="0"
DEBUG="false"
CLEAN="false"
BOOTSTRAP="false"
INSTALL_P="false"
PYTHON_BINDINGS="false"

CONFIGURE_EXTRA_ARGS=()
MAKE_EXTRA_ARGS=()

ORIG_NUM_ARGS=$#

while [ "$#" -ne 0 ]
do
    case "$1" in
	--bootstrap)
	    BOOTSTRAP="true"
	    shift
	    ;;
	--clean)
	    CLEAN="true"
	    shift
	    ;;
	--debug)
	    DEBUG="true"
	    shift
	    ;;
	-h|--help)
	    print_help
	    exit 0
	    ;;
	-j)
	    JOBS="$2"
	    [ ! -z "$JOBS" ] || die "Option -j requires an argument."

	    shift
	    shift
	    ;;
	-j[0-9]*)
	    JOBS=`echo "$1" | cut -c 3-`
	    shift
	    ;;
	-v|--verbose)
	    VERBOSITY="1"
	    shift
	    ;;
	--local|--local-install)
	    PREFIX="$PWD/install"
	    LOCAL_INSTALL="true"
	    shift
	    ;;
	--install-p)
	    INSTALL_P="true"
	    shift
	    ;;
	--prefix=*)
	    PREFIX=`echo "$1" | cut -c 10-`
	    shift
	    ;;
	--enable-python)
	    PYTHON_BINDINGS="true"
	    shift
	    ;;
	*)
	    die "Unknown flag \"$1\"."
	    ;;
    esac
done

BOOTSTRAP_SUBPROJECTS="aftermath aftermath-convert aftermath-dump libaftermath-core libaftermath-render libaftermath-trace"

if [ $PYTHON_BINDINGS = "true" ]
then
    BOOTSTRAP_SUBPROJECTS="$BOOTSTRAP_SUBPROJECTS language-bindings/python/libaftermath-core"
fi

if [ $CLEAN = "true" ]
then
    rm -rf build

    for SUBPROJECT in $BOOTSTRAP_SUBPROJECTS
    do
	(cd "./$SUBPROJECT" ; ./bootstrap.sh --clean )
    done

    if [ $ORIG_NUM_ARGS -eq 1 ]
    then
	exit 0
    fi
fi

for SUBPROJECT in $BOOTSTRAP_SUBPROJECTS
do
    if [ ! -f "./$SUBPROJECT/configure" -o $BOOTSTRAP = "true" ]
    then
	printf "%s" "Running bootstrap for $SUBPROJECT... "
	(cd "./$SUBPROJECT" ; ./bootstrap.sh ) > log 2>&1
	RETVAL=$?

	if [ $RETVAL -eq 0 ]
	then
	    echo "done."
	else
	    echo "failed!"
	    echo "Boostrap failed for $SUBPROJECT:" >&2
	    cat log >&2
	    exit 1
	fi
    fi
done

[ ! -z "$PREFIX" ] && CONFIGURE_EXTRA_ARGS+=("--prefix=$PREFIX")
[ ! -z "$JOBS" ] && MAKE_EXTRA_ARGS+=("-j$JOBS")
[ ! -z "$VERBOSITY" ] && MAKE_EXTRA_ARGS+=("V=$VERBOSITY")
[ $DEBUG = "true" ] && CONFIGURE_EXTRA_ARGS+=("CFLAGS=-g -O0")
[ $DEBUG = "true" ] && CONFIGURE_EXTRA_ARGS+=("CXXFLAGS=-g -O0")
[ $INSTALL_P = "true" ] && CONFIGURE_EXTRA_ARGS+=("INSTALL=`which install` -p")

do_configure libaftermath-core "${CONFIGURE_EXTRA_ARGS[@]}"
do_make libaftermath-core "${MAKE_EXTRA_ARGS[@]}"

CONFIGURE_LIBTRACE_ARGS=$CONFIGURE_EXTRA_ARGS
CONFIGURE_LIBTRACE_ARGS+=("--with-aftermath-core-sourcedir=$PWD/libaftermath-core"
			  "--with-aftermath-core-builddir=$PWD/build/libaftermath-core")
do_configure libaftermath-trace "${CONFIGURE_LIBTRACE_ARGS[@]}"
do_make libaftermath-trace "${MAKE_EXTRA_ARGS[@]}"

CONFIGURE_EXTRA_ARGS+=("--with-aftermath-core=$PREFIX")

for SUBPROJECT in aftermath-convert \
		      aftermath-dump \
		      libaftermath-render
do
    do_configure $SUBPROJECT "${CONFIGURE_EXTRA_ARGS[@]}"
    do_make $SUBPROJECT "${MAKE_EXTRA_ARGS[@]}"
done

CONFIGURE_EXTRA_ARGS+=("--with-aftermath-render=$PREFIX")
do_configure aftermath "${CONFIGURE_EXTRA_ARGS[@]}"
do_make aftermath "${MAKE_EXTRA_ARGS[@]}"

if [ $PYTHON_BINDINGS = "true" ]
then
    CONFIGURE_PYLIBCORE_ARGS=$CONFIGURE_EXTRA_ARGS
    CONFIGURE_PYLIBCORE_ARGS+=("--with-aftermath-core=$PREFIX"
			       "--with-aftermath-core-sourcedir=$PWD/libaftermath-core"
			       "--with-aftermath-core-builddir=$PWD/build/libaftermath-core")
    do_configure language-bindings/python/libaftermath-core "${CONFIGURE_PYLIBCORE_ARGS[@]}"
    do_make language-bindings/python/libaftermath-core "${MAKE_EXTRA_ARGS[@]}"
fi

echo
echo "*********************************************************************"
echo "* Build finished successfully!                                      *"
echo "*********************************************************************"
echo
echo "If not already set, please add"
echo
echo "  $PREFIX/lib"
echo
echo "to LD_LIBRARY_PATH and"
echo
echo "  $PREFIX/bin"
echo
echo "to PATH, e.g., by executing:"
echo
echo "  export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$PREFIX/lib"
echo "  export PATH=\$PATH:$PREFIX/bin"
echo

if [ $PYTHON_BINDINGS = "true" ]
then
    PYTHON_SITEPACKAGE_DIR=`echo $PREFIX/lib/python*/site-packages`
    echo "Python packages have been installed to $PYTHON_SITEPACKAGE_DIR."
    echo "Please set PYTHONPATH accordingly, e.g., by executing:"
    echo
    echo "  export PYTHONPATH=$PYTHON_SITEPACKAGE_DIR"
    echo
fi
