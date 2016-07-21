#!/bin/sh

. $(dirname "$0")/lib.sh

print_usage() {
    echo "Usage: $0 [OPTION]... REVISION"
    echo "Checks if a specific git revision of the project builds, installs and if its"
    echo "tests run correctly."
    echo
    echo "Options:"
    echo "  -d DIRECTORY               Use DIRECTORY as the build directory"
    echo "  -v, --verbose              Verbose output"
    echo "  -j N, --jobs=N             Build using N jobs"
    echo "  -f, --force-overwrite      Overwrite build directory if it already exists"
    echo "  -k, --keep                 Keep files; do not remove build directory when done"
    echo "  -h, --help                 Print usage"
    echo
    echo "Exit status:"
    echo " 0 Revision build, installs and tests pass"
    echo " 1 An error occured either in the build, install or test phase"
    exit 0
}

[ -d ".git" ] || die "Please run this script from the main directory"

BUILDFLAGS=""

while [ $# -ne 0 ]
do
    case "$1" in
	-d*)
	    BUILD_DIR=$(parse_opt "$1" "$2")
	    $(parse_opt_shift "$1")
	    ;;
	-v|--verbose)
	    VERBOSE="true"
	    ;;
	-f|--force-overwrite)
	    FORCE_OVERWRITE="true"
	    ;;
	-k|--keep)
	    KEEP_FILES="true"
	    ;;
	-h|--help)
	    print_usage $0
	    ;;
	-j*|--jobs=*)
	    JOBS=$(parse_opt "$1" "$2")
	    $(parse_opt_shift "$1")
	    BUILDFLAGS="$BUILDFLAGS -j$JOBS"
	    ;;
	-*)
	    die "Unknown flag \"$1\". Aborting."
	    ;;
	*)
	    [ -z "$REV" ] || die "Revision already specified. Aborting."
	    REV="$1"
	    ;;
    esac

    shift
done

OLD_PWD="$PWD"

[ ! -z "$REV" ] || die "Please specify a revision to test"

SHORT_REV=$(git rev-parse --short "$REV" 2>&1 ) || die "Could not find revision \"$REV\""

[ ! -z "$BUILD_DIR" ] || BUILD_DIR="build-tmp/$SHORT_REV"

echo_verbose "Building in $BUILD_DIR"

[ ! -e "$BUILD_DIR" ] || [ ! -z $FORCE_OVERWRITE ] || die "$BUILD_DIR esists. Use -f to force overwrite. Aborting. "
[ ! -z $FORCE_OVERWRITE ] && rm -rf "$BUILD_DIR"

mkdir -p "$BUILD_DIR" >/dev/null 2>&1 || die "Could not create \"$BUILD_DIR\""

echo_verbose_n "Checking out files... "
git archive "$REV" | tar x -C "$BUILD_DIR" >> "$BUILD_DIR/build-log" 2>&1 || die "Could not check out files. Check $BUILD_DIR/build-log."
echo_verbose "done."

echo_verbose_n "Checking headers... "
for header in "$BUILD_DIR"/src/*.h
do
    ./scripts/check-header.sh -v "$header" >> "$BUILD_DIR/build-log" 2>&1 || die "Header check failed. Check $BUILD_DIR/build-log."
done
echo_verbose "done."

cd "$BUILD_DIR"

echo_verbose_n "Executing bootstrap script... "
./bootstrap >> build-log 2>&1 || die "Could not execute bootstrap script. Check $BUILD_DIR/build-log."
echo_verbose "done."

INSTALL_DIR="$PWD/install"

echo_verbose_n "Executing configure... "
./configure --prefix="$INSTALL_DIR" >> build-log 2>&1 || die "Could not execute configure. Check $BUILD_DIR/build-log."
echo_verbose "done."

echo_verbose_n "Building... "
make $BUILDFLAGS >> build-log 2>&1 || die "Could not build project. Check $BUILD_DIR/build-log."
echo_verbose "done."

echo_verbose_n "Running tests... "
make check VERBOSE=1 >> build-log 2>&1 || die "Tests failed. Check $BUILD_DIR/build-log."
echo_verbose "done."

echo_verbose_n "Installing... "
make install >> build-log 2>&1 || die "Could not install project. Check $BUILD_DIR/build-log."
echo_verbose "done."

echo_verbose_n "Making dist... "
make distcheck >> build-log 2>&1 || die "Could not build from distribution archive. Check $BUILD_DIR/build-log."
echo_verbose "done."

echo_verbose "Revision built, tested and installed successfully."

cd "$OLD_PWD"
[ ! -z "$KEEP_FILES" ] || rm -rf "$BUILD_DIR" || die "Could not remove build directory"
