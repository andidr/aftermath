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
