#!/bin/sh

die() {
    echo "$1" >&2
    exit 1
}

echo_verbose() {
    [ ! -z $VERBOSE ] && echo "$1"
}

echo_verbose_n() {
    [ ! -z $VERBOSE ] && echo -n "$1"
}

substr () {
    IGNORE="$2"
    echo "$1" | sed "s/.\{$IGNORE\}//"
}

longopt_val () {
    echo "$1" | sed "s/^[^=]*=//"
}

parse_opt() {
    CURR="$1"
    FOLLOWING="$2"

    case "$CURR" in
	--*)
	    longopt_val "$CURR"
	    ;;
	-[a-zA-Z])
	    echo "$FOLLOWING"
	    ;;
	-[a-zA-Z]*)
	    substr "$CURR" 2
	    ;;
    esac
}

parse_opt_shift() {
    CURR="$1"

    case "$CURR" in
	-[a-zA-Z])
	    echo "shift"
	    ;;
    esac
}
