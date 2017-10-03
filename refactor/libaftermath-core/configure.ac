AC_INIT([libaftermath-core], [0.1])
AC_CONFIG_SRCDIR([src/ansi_extras.c])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([subdir-objects tar-pax])

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

AC_OUTPUT([Makefile headers/Makefile])
