#
# Checks if a library is available at a specific location. The first
# argument is the library's base name without its extension and
# without the lib prefix (e.g., foo and not libfoo.a). The second
# argument is the full path to the directory that is searched for the
# library. While searching, the macro extends the name with the the
# suffixes .a, .so, .lib and .dll until the library is found. If the
# library cannot be found, an error is generated.
#
AC_DEFUN([CHECK_LIB_IN_PATH], [
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

#
# Checks if a header file is available at a specific location. The
# first argument is the header file's base name and the second
# argument is the full path to the directory that is searched for the
# header. If the header file cannot be found, an error is generated.
#
AC_DEFUN([CHECK_HEADER_IN_PATH], [
	HEADER=$1
	HEADER_PATH=$2

	echo -n "checking for $HEADER in $HEADER_PATH... "

	if test -f "$HEADER_PATH/$HEADER"
	then
		echo "yes"
	else
		echo "no"
		AC_ERROR([Could not find $HEADER in $HEADER_PATH: "$HEADER_PATH/$HEADER"])
	fi
])

#
# Adds an option --with-<package>-libdir=DIR to the configure
# script. Arguments:
#
#  $1: Name of the package the library belongs to
#  $2: The base name of the library without file extension and without
#      the lib prefix
#  $3: A function from the library
#
# If the option is set, the directory DIR is searched for the library
# using different extensions (.a .so .lib .dll). If the library cannot
# be found, an error is generated. Otherwise, a variable XXX_LIBS with
# the required linker flags and XXX_INCLUDE_DIR with DIR are defined
# and substituted (where XXX is the uppercase package name).
#
AC_DEFUN([CHECK_LIB_WITH],
	[
		PKGNAME=$1
		LIBNAME=$2
		FUNCTION=$3
		translit([[$1]], [a-z-], [A-Z_])_LIBS="-l$2"

		AC_ARG_WITH($1-libdir,
			AS_HELP_STRING([ --with-$1-libdir=DIR],
					[Use $1 libraries from DIR]),
			LDFLAGS="-L$withval $LDFLAGS"
			WITH_LIBDIR=$withval
			translit([[$1]], [a-z-], [A-Z_])_LIB_DIR="$withval")

		if test "x$WITH_LIBDIR" != "x"
		then
			CHECK_LIB_IN_PATH($LIBNAME, $WITH_LIBDIR)
		fi

		AC_CHECK_LIB($LIBNAME, $FUNCTION,
				[AC_DEFINE_UNQUOTED(HAVE_LIB[]translit([[$2]], [a-z-], [A-Z_]),1,[Defined if you have the $1 library])],
				AC_MSG_ERROR([Required library lib$LIBNAME of package $PKGNAME does not provide function $3]))

		AC_SUBST(translit([[$1]], [a-z-], [A-Z_])_LIB_DIR)
		AC_SUBST(translit([[$1]], [a-z-], [A-Z_])_LIBS)
		WITH_LIBDIR=""
	])

#
# Adds an option --with-<package>-includedir=DIR to the configure
# script. Arguments:
#
#  $1: Name of the package the header file belongs to
#  $2: A header file
#
# If the option is set, the directory DIR is searched for the header
# file. If the file cannot be found, an error is generated. Otherwise,
# a variable XXX_INCLUDES with the required compiler flags and
# XXX_INCLUDE_DIR with DIR are defined and substituted (where XXX is
# the uppercase package name).
#
AC_DEFUN([CHECK_HEADER_WITH],
	[
		PKGNAME=$1
		HEADER=$2

		AC_ARG_WITH($1-includedir,
			AS_HELP_STRING([--with-$1-includedir=DIR],
					[Use $1 headers from DIR]),
			CPPFLAGS="-I$withval $CPPFLAGS"
			WITH_INCDIR=$withval
			translit([[$1]], [a-z-], [A-Z_])_INCLUDE_DIR="$withval")

		if test "x$WITH_INCDIR" != "x"
		then
			CHECK_HEADER_IN_PATH($HEADER, $WITH_INCDIR)
		fi

		AC_CHECK_HEADER([$HEADER], , AC_MSG_ERROR([Could not find header file $HEADER of package $PKGNAME]))

		AC_SUBST(translit([[$1]], [a-z-], [A-Z_])_INCLUDE_DIR)
		AC_SUBST(translit([[$1]], [a-z-], [A-Z_])_INCLUDES)

		WITH_INCDIR=""
	])

#
# Adds the options --with-<package>=DIR
# --with-<package>-includedir=DIR, and --with-<package>-libdir=DIR to
# the configure script. Arguments:
#
#  $1: Name of the package the library belongs to
#  $2: The base name of the library without file extension and without
#      the lib prefix
#  $3: A header file
#  $4: A function from the library
#
# If the option is set, the directory DIR/include is searched for the
# header file and the directory DIR/lib is searched for the library
# using different extensions (.a .so .lib .dll). If the files cannot
# be found, an error is generated. Otherwise, the variable
# XXX_INCLUDES with the required compiler flags and the variable
# XXX_LIBS with the required linker flags are defined and substituted
# (where XXX is the uppercase package name). XXX_INCLUDE_DIR will contain
# the path to the package's header files and XXX_LIB_DIR will contain the
# path to the package's libraries.
#
AC_DEFUN([CHECK_LIB_AND_HEADER_WITH], [
	PKGNAME=$1
	LIBNAME=$2
	HEADER=$3
	FUNCTION=$4

	AC_ARG_WITH($1,
		AS_HELP_STRING([--with-$1=DIR],
			[Use $1 headers from DIR/include and libraries from DIR/lib]),
		WITH_DIR="$withval";
		WITH_LIBDIR="$withval/lib";
		WITH_INCDIR="$withval/include";
		CPPFLAGS="-I$WITH_INCDIR $CPPFLAGS";
		LDFLAGS="-L$withval/lib $LDFLAGS"
		translit([[$1]], [a-z-], [A-Z_])_LIB_DIR="$withval/lib"
		translit([[$1]], [a-z-], [A-Z_])_INCLUDE_DIR="$withval/include")

	if test "x$WITH_DIR" != "x"
	then
		CHECK_LIB_IN_PATH($LIBNAME, $WITH_LIBDIR)
		AC_SUBST(translit([[$1]], [a-z-], [A-Z_])_LIBS)

		CHECK_HEADER_IN_PATH($HEADER, $WITH_INCDIR)
		AC_SUBST(translit([[$1]], [a-z-], [A-Z_])_INCLUDES)
	fi

	WITH_LIBDIR=""
	WITH_INCDIR=""
	WITH_DIR=""

	CHECK_HEADER_WITH($1, $3)
	CHECK_LIB_WITH($1, $2, $4)
])
