#!/bin/sh

if [ $# -ne 1 ]
then
    echo "Usage: $0 file" >&2
    exit 1
fi

EXECUTABLE="$1"

if [ ! -f "$EXECUTABLE" ]
then
    echo "Could not find $EXECUTABLE" >&2
    exit 1
fi

nm -l "$EXECUTABLE" | grep '[0-9A-Fa-f]* [tT]' | sed 's/:/ /g'
