#!/bin/sh

if [ "x$1" = "x--clean" ]
then
	if [ -f Makefile ]
	then
		make distclean
	fi
	rm -rf aclocal.m4 \
	   Makefile.in \
	   depcomp Makefile.in \
	   autom4te.cache \
	   compile \
	   configure \
	   install-sh \
	   missing \
	   config.guess \
	   config.sub \
	   config.h.in \
	   ltmain.sh \
	   m4/libtool.m4 \
	   m4/lt~obsolete.m4 \
	   m4/ltoptions.m4 \
	   m4/ltsugar.m4 \
	   m4/ltversion.m4
else
	aclocal && \
	autoconf && \
	automake --gnu --add-missing --copy
fi
