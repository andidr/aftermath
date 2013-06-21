#!/bin/sh

if [ "$UNIT_TESTS_TIMER_STARTED " != "true " ]
then
	chmod +x $0
	export UNIT_TESTS_TIMER_STARTED=true
	time ./$0 $@
	exit $?
fi

for test in $@
do
	./$test
	if [ $? != 0 ]
	then
		echo
		echo -e "\033[49;0;31m AT LEAST ONE TEST FAILED! ABORTING... \033[0m"
		exit $?
	fi
done

echo -e "\033[49;0;32mCONGRATULATIONS! ALL TESTS OK. \033[0m"
