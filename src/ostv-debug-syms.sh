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

nm -l "$EXECUTABLE" | grep '[0-9A-Fa-f]* [tT]' | \
    sed -e 's/:/ /g' -e 's/[\t]/ /g' -e 's/[ ]\{1,\}/ /g' | \
    grep '[0-9A-Fa-f]\{1,\} [tT] [^ ]\{1,\} [^ ]\{1,\} [0-9]\{1,\}'
