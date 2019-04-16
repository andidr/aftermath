AC_INIT([libaftermath-core-py], [0.5])
AC_CONFIG_SRCDIR([src/templates/array.tpl.py])
AC_CONFIG_MACRO_DIRS([m4])

m4_include([m4/with-check.m4])
m4_include([m4/python.m4])
m4_include([m4/custom-program.m4])

AM_INIT_AUTOMAKE([subdir-objects tar-pax])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_C_PROTOTYPES

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_VPRINTF

# Check for python and python modules
CHECK_CUSTOM_PROG(python)
AC_PYTHON_MODULE(argparse, true)
AC_PYTHON_MODULE(cffi, true)
AC_PYTHON_MODULE(copy, true)
AC_PYTHON_MODULE(inspect, true)
AC_PYTHON_MODULE(jinja2, true)
AC_PYTHON_MODULE(os, true)
AC_PYTHON_MODULE(pkgutil, true)
AC_PYTHON_MODULE(re, true)
AC_PYTHON_MODULE(setuptools, true)
AC_PYTHON_MODULE(sys, true)
AC_PYTHON_MODULE(wheel, true)

CHECK_LIB_AND_HEADER_WITH([aftermath-core], [aftermath-core],
	[aftermath/core/base_types.h], [am_dsk_load_trace])

AC_ARG_WITH(aftermath-core-builddir,
	AS_HELP_STRING([ --with-aftermath-core-builddir=DIR],
			[Use aftermath-core build directory DIR]),
	AFTERMATH_CORE_BUILDDIR=$withval)

if test -z "$AFTERMATH_CORE_BUILDDIR"
then
	AC_ERROR([No build directory for libaftermath-core specified. Please consider passing one using --with-aftermath-core-builddir=])
fi

AC_SUBST(AFTERMATH_CORE_BUILDDIR)
AC_CHECK_FILE([$AFTERMATH_CORE_BUILDDIR/src/in_memory.h],, AC_ERROR([Could not find file src/in_memory.h in libaftermath-core build directory]))
AC_CHECK_FILE([$AFTERMATH_CORE_BUILDDIR/src/base_types.h],, AC_ERROR([Could not find file src/base_types.h in libaftermath-core build directory]))

AC_ARG_WITH(aftermath-core-sourcedir,
	AS_HELP_STRING([ --with-aftermath-core-sourcedir=DIR],
			[Use aftermath-core source directory DIR]),
	AFTERMATH_CORE_SOURCEDIR=$withval)

if test -z "$AFTERMATH_CORE_SOURCEDIR"
then
	AC_ERROR([No src directory for libaftermath-core specified. Please consider passing one using --with-aftermath-core-sourcedir=])
fi

AC_CHECK_PROGS(PIP, [pip])

if test -z "$PIP"
then
	AC_MSG_ERROR([Could not find pip.])
fi

AC_SUBST(AFTERMATH_CORE_SOURCEDIR)
AC_CHECK_FILE([$AFTERMATH_CORE_SOURCEDIR/src/default_trace_array_registry.c],, AC_ERROR([Could not find file src/default_trace_array_registry.c in libaftermath-core source directory]))

AC_OUTPUT([Makefile src/package/build.py src/package/setup.py])
