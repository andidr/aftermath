#!/bin/sh

. $(dirname "$0")/lib.sh

print_usage() {
    echo "Usage: $0 [OPTION]... START_REVISION [END_REVISION]"
    echo "Checks if all revisions between START_REVISION and END_REVISION build,"
    echo "install and if tests run correctly."
    echo
    echo "If the end revision is omitted HEAD is assumed to be the end revision."
    echo
    echo "Options:"
    echo "  -p PREFIX, --prefix=PREFIX Use PREFIX for the build directory"
    echo "  -v, --verbose              Verbose output"
    echo "  -vv, --very-verbose        Also use verbose output when checking a single revision"
    echo
    echo "For more options and flags see ./scripts/check-revision.sh --help"
    echo
    echo "Exit status:"
    echo " 0 All revisions build, install and pass tests"
    echo " 1 At least one of the revision does not build / install / pass tests"
    exit 0
}

[ -d ".git" ] || die "Please run this script from the main directory"

FLAGS=""

while [ $# -ne 0 ]
do
    case "$1" in
	-v|--verbose)
	    VERBOSE="true"
	    ;;
	-vv|--very-verbose)
	    VERBOSE="true"
	    VERY_VERBOSE="true"
	    FLAGS="$FLAGS -v"
	    ;;
	-f|--force-overwrite)
	    FORCE_OVERWRITE="true"
	    FLAGS="$FLAGS -f"
	    ;;
	-k|--keep)
	    KEEP_FILES="true"
	    FLAGS="$FLAGS -k"
	    ;;
	-h|--help)
	    print_usage $0
	    ;;
	-p*|--prefix=*)
	    BUILD_PREFIX=$(parse_opt "$1" "$2")
	    $(parse_opt_shift "$1")
	    ;;
	-j*|--jobs=*)
	    JOBS=$(parse_opt "$1" "$2")
	    $(parse_opt_shift "$1")
	    echo $(parse_opt_shift "$1")
	    FLAGS="$FLAGS -j$JOBS"
	    ;;
	-*)
	    die "Unknown flag \"$1\". Aborting."
	    ;;
	*)
	    [ -z "$REV_END" ] || die "Start and end revision already specified. Aborting."
	    [ ! -z "$REV_START" -a -z "$REV_END" ] && REV_END="$1"
	    [ -z "$REV_START" ] && REV_START="$1"
	    ;;
    esac

    shift
done

[ ! -z "$REV_START" ] || die "Please specify a start revision."
[ ! -z "$REV_END" ] || REV_END="HEAD"

SHORT_REV_START=$(git rev-parse --short "$REV_START" 2>&1 ) || die "Could not find revision \"$REV_START\""
SHORT_REV_END=$(git rev-parse --short "$REV_END" 2>&1 ) || die "Could not find revision \"$REV_END\""

echo_verbose "Checking revisions $SHORT_REV_START to $SHORT_REV_END"
REVISIONS=$(git rev-list "$SHORT_REV_START^..$SHORT_REV_END") || die "Could not list revisions between \"$REV_START\" and \"$REV_END\""

i=1
n=$(echo "$REVISIONS" | wc -l)

for REV in $REVISIONS
do
    [ ! -z "$VERBOSE" -a -z "$VERY_VERBOSE" ] &&  echo_verbose_n "[$i/$n] Checking $REV... "
    [ ! -z "$VERBOSE" -a ! -z "$VERY_VERBOSE" ] &&  echo_verbose "[$i/$n] Checking $REV... "

    [ ! -z "$BUILD_PREFIX" ] && EXTRA_FLAGS="-d \"$BUILD_PREFIX/$REV/\""

    ./scripts/check-revision.sh $FLAGS $EXTRA_FLAGS $REV || die "Check for $REV failed."

    [ ! -z "$VERBOSE" -a -z "$VERY_VERBOSE" ] && echo_verbose "done."
    i=$(($i + 1))
done
