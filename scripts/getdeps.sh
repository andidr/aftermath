#!/bin/sh

die() {
    echo "$1" >&2
    exit 1
}

die_unsupported() {
    echo "Could not determine dependencies for OS \"$PRETTY_NAME\"." >&2
    echo "Please install dependencies manually." >&2
    exit 1
}

die_usage() {
    echo "Usage: $0 OPTIONS"
    echo "Auto-detect OS and install dependences using the package manager."
    echo ""
    echo "OPTIONS"
    echo "  -b, --build-deps  Install dependencies for compilation [default]"
    echo "  -d, --dry-run     Do not actuall install dependencies, only show the command"
    echo "  -r, --run-deps    Install dependencies for running the program"
    echo "  -s, --sudo        Use sudo to invoke package manager"
    exit 1
}

DEP_TYPE="build"
COMMAND_PREFIX=""
DRY_RUN="false"

while [ $# -ne 0 ]
do
    case "$1" in
	-b|--build-deps)
	    DEP_TYPE="build"
	    ;;
	-d|--dry-run)
	    DRY_RUN="true"
	    ;;
	-r|--run-deps)
	    DEP_TYPE="run"
	    ;;
	-s|--sudo)
	    COMMAND_PREFIX="sudo"
	    ;;
	*)
	    die "Unknown option \"$1\". Aborting."
	    ;;
    esac

    shift
done

[ -f "/etc/os-release" ] || die "Cannot find /etc/os-release and thus cannot detect OS."

. "/etc/os-release"

DISTRO_NAME=""
DISTRO_RELEASE=""
case "$NAME" in
    Debian*)
	DISTRO_NAME="debian"

	if [ "x$VERSION_ID" = "x8" ]
	then
	    DISTRO_RELEASE="jessie"
	fi
	;;
    *)
	echo "NAME IS $NAME"
	;;
esac

[ ! -z "$DISTRO_NAME" ] || die_unsupported
[ ! -z "$DISTRO_RELEASE" ] || die_unsupported

if [ $DEP_TYPE = "run" ]
then
    DISTRO_DEPFILE="distros/$DISTRO_NAME/deps/$DISTRO_RELEASE.deps"
else
    DISTRO_DEPFILE="distros/$DISTRO_NAME/deps/$DISTRO_RELEASE.build-deps"
fi

DEPS=`cat "$DISTRO_DEPFILE"`

case "$DISTRO_NAME" in
    debian|ubuntu)
	COMMAND="$COMMAND_PREFIX apt-get install $DEPS"
	;;
esac

if [ $DRY_RUN = "true" ]
then
    echo $COMMAND
else
    $COMMAND
fi
