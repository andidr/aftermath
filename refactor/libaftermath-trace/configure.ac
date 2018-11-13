AC_INIT([libaftermath-trace], [0.1])
AC_CONFIG_SRCDIR([src/buffered_event_collection.h])
AC_CONFIG_MACRO_DIRS([m4])

m4_include([m4/with-check.m4])

AM_INIT_AUTOMAKE([subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_C_PROTOTYPES

LT_INIT

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_VPRINTF

#
# Check for libaftermath-core sources
#
AC_ARG_WITH(aftermath-core-sourcedir,
	AS_HELP_STRING(
		[ --with-aftermath-core-sourcedir=DIR],
		[Use libaftermath-core source files from DIR]),

	AFTERMATH_CORE_SOURCEDIR="$withval"
)

# If the libaftermath-core build is a vpath build, generated
# files are created in the build directory.
AC_ARG_WITH(aftermath-core-builddir,
	AS_HELP_STRING(
		[ --with-aftermath-core-builddir=DIR],
		[Use generated libaftermath-core source files from DIR]),

	AFTERMATH_CORE_BUILDDIR="$withval"
)

if test "x$AFTERMATH_CORE_SOURCEDIR" = "x"
then
	AC_MSG_ERROR([Could not find libaftermath-core sources. Please indicate the base directory using --with-aftermath-core-sourcedir=DIR.])

	# If no vpath builddir is specified, assume that
	# libaftermath-core was directly built in its source directory
	if test "x$AFTERMATH_CORE_BUILDDIR" = "x"
	then
		AFTERMATH_CORE_BUILDDIR="$AFTERMATH_CORE_SOURCEDIR"
	fi
fi

AC_SUBST(AFTERMATH_CORE_SOURCEDIR)
AC_SUBST(AFTERMATH_CORE_BUILDDIR)

for srcfile in src/convert.h \
		src/safe_alloc.h \
		src/write_buffer.c \
		src/write_buffer.h
do
	AC_CHECK_FILE([$AFTERMATH_CORE_SOURCEDIR/$srcfile],,
		AC_MSG_ERROR([Could not find required file $srcfile in libaftermath-core source directory.]))
done

for srcfile in src/base_types.h \
		src/on_disk_structs.h \
		src/on_disk_write_to_buffer.h
do
	AC_CHECK_FILE([$AFTERMATH_CORE_BUILDDIR/$srcfile],,
		AC_MSG_ERROR([Could not find required file $srcfile in the libaftermath-core build directory.]))
done

AC_OUTPUT([Makefile headers/Makefile])
