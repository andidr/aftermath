#!/bin/bash

function check_leak_summary()
{
	grep "$LEAK_SUMMARY_LINE" "$VALGRIND_FILE" > /dev/null
	
	if [ $? != 0 ]
	then
		exit 1
	fi
}

if [ $# != 1 ]
then
	echo "Usage: $0 <valgrind_file>"
	exit 1;
fi

VALGRIND_FILE=$1

if [ ! -f "$VALGRIND_FILE" ]
then
	echo "Could not find $VALGRIND_FILE"
	exit 1;
fi

LEAK_SUMMARY_LINE="ERROR SUMMARY: 0 errors from 0 contexts"
check_leak_summary

grep "LEAK SUMMARY:" "$VALGRIND_FILE" > /dev/null
if [ $? = 0 ]
then
	LEAK_SUMMARY_LINE="definitely lost: 0 bytes in 0 blocks"
	check_leak_summary
	LEAK_SUMMARY_LINE="possibly lost: 0 bytes in 0 blocks"
	check_leak_summary
	LEAK_SUMMARY_LINE="still reachable: 0 bytes in 0 blocks"
	check_leak_summary
fi

exit 0
