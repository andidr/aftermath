#!/bin/sh

LINE_READ_STATUS=0
while test $LINE_READ_STATUS -eq 0
do
	read LINE
	LINE_READ_STATUS=$?

	if test $LINE_READ_STATUS -eq 0
	then
		LINE_FILTERED=$(echo "$LINE" | $SED "s{.*\(-o \([A-Za-z0-9_/-]*\.o\)\).*{[CC] \2{")
		LINE_FILTERED_KNOWN=$(echo "$LINE_FILTERED" | $EGREP "^\[[A-Z]+\]")
		if test ! -z "$LINE_FILTERED_KNOWN"; then KNOWN=true; else KNOWN=false; fi

		if test $KNOWN = false
		then
			LINE_FILTERED=$(echo "$LINE" | $SED "s{.*--mode=link.*\(-o \([A-Za-z0-9_/.-]*\)\).*{[LD] \2{")
			LINE_FILTERED_KNOWN=$(echo "$LINE_FILTERED" | $EGREP "^\[[A-Z]+\]")
			if test ! -z "$LINE_FILTERED_KNOWN";
			then
				LINE=""
			fi
		fi

		if test $KNOWN = false
		then
			LINE_FILTERED=$(echo "$LINE" | $SED "s{.*\(-o \([A-Za-z0-9_/.-]*\)\).*{[LD] \2{")
			LINE_FILTERED_KNOWN=$(echo "$LINE_FILTERED" | $EGREP "^\[[A-Z]+\]")
			if test ! -z "$LINE_FILTERED_KNOWN"; then KNOWN=true; else KNOWN=false; fi
		fi

		if test $KNOWN = false
		then
			LINE_FILTERED=$(echo "$LINE" | $SED "s{.*$AR.* \(\([A-Za-z0-9_/-]*\)\.a\).*{[AR] \1{")
			LINE_FILTERED_KNOWN=$(echo "$LINE_FILTERED" | $EGREP "^\[[A-Z]+\]")
			if test ! -z "$LINE_FILTERED_KNOWN"; then KNOWN=true; else KNOWN=false; fi
		fi

		if test $KNOWN = true
		then
			#echo
			#echo "$LINE"
			echo "$LINE_FILTERED"
		fi
	fi
done