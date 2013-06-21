AC_DEFUN([CHECK_CUSTOM_PROG],
	[
		PROG=$1
		PROG_UPPER=translit([[$1]], [a-z], [A-Z])
		
		AC_PATH_PROG(translit([[$1]], [a-z], [A-Z]), $1)
		if test "x$[]translit([[$1]], [a-z], [A-Z])" = x
		then
			AC_ERROR([Could not find program $PROG.])
		fi
	])

AC_DEFUN([CHECK_LIB_IN_PATH],
	[
		LIB_NAME=$1
		LIB_PATH=$2
		
		echo -n "checking for lib$LIB_NAME in $LIB_PATH... "
		
		found=0
		extensions="a so lib dll"
		extensions_joined=""
		for ext in $extensions
		do
			extensions_joined="$extensions_joined$ext,"
			if test -f "$LIB_PATH/lib$LIBNAME.$ext"
			then
				found=1
			fi
		done
		
		if test -f "$LIB_PATH/lib$LIBNAME"
		then
			found=1
		fi
		
		if test $found -ne 1
		then
			echo "no"
			AC_ERROR([Could not find lib$LIB_NAME.{$extensions_joined} in $LIB_PATH])
		fi
		
		echo "yes"
	])

AC_DEFUN([CHECK_HEADER_IN_PATH],
	[
		HEADER=$1
		HEADER_PATH=$2
		
		echo -n "checking for $HEADER in $HEADER_PATH... "
		
		if test -f "$HEADER_PATH/$HEADER"
		then
			echo "yes"
		else
			echo "no"
			AC_ERROR([Could not find $HEADER in $HEADER_PATH])
		fi
		
		
	])

AC_DEFUN([CHECK_LIB_WITH],
	[
		PKGNAME=$1
		LIBNAME=$2
		FUNCTION=$3
		translit([[$1]], [a-z], [A-Z])_LIBS="-l$2"

		AC_ARG_WITH($1-libdir,
				[  --with-$1-libdir=DIR		use $1 libraries from DIR],
				LDFLAGS="-L$withval $LDFLAGS"
				WITH_LIBDIR=$withval)
		
		if test "x$WITH_LIBDIR" != "x"
		then
			CHECK_LIB_IN_PATH($LIBNAME, $WITH_LIBDIR)
		fi
		
		AC_CHECK_LIB($LIBNAME, $FUNCTION,
				[AC_DEFINE_UNQUOTED(HAVE_LIB[]translit([[$LIBNAME]], [a-z], [A-Z]),1,[Defined if you have the $1 library])],
				AC_MSG_ERROR([Required library $LIBNAME of package $PKGNAME not available]))

		AC_SUBST(translit([[$1]], [a-z], [A-Z])_LIBS)
		WITH_LIBDIR=""
	])

AC_DEFUN([CHECK_HEADER_WITH],
	[
		PKGNAME=$1
		HEADER=$2

		AC_ARG_WITH($1-includedir,
				[  --with-$1-includedir=DIR		use $1 headers from DIR],
				CPPFLAGS="-I$withval $CPPFLAGS"
				WITH_INCDIR=$withval)
		
		if test "x$WITH_INCDIR" != "x"
		then
			CHECK_HEADER_IN_PATH($HEADER, $WITH_INCDIR)
		fi
		
		AC_CHECK_HEADER([$HEADER], , AC_MSG_ERROR([Could not find $HEADER of package $PKGNAME]))

		AC_SUBST(translit([[$1]], [a-z], [A-Z])_INCLUDES)
		
		WITH_INCDIR=""
	])