AC_INIT([aftermath], [unreleased])
AC_CONFIG_SRCDIR([src/aftermath.c])
AM_INIT_AUTOMAKE([subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
m4_include([m4/with_check.m4])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_C_PROTOTYPES
AC_PROG_GREP
AC_PROG_SED
CHECK_CUSTOM_PROG(mkdir)
CHECK_CUSTOM_PROG(glib-genmarshal)

# Checks for libraries.
CHECK_LIB_WITH(pthread, pthread, pthread_create)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER(ctype.h, , AC_ERROR([Could not find ctype.h on your system.]))
AC_CHECK_HEADER(errno.h, , AC_ERROR([Could not find errno.h on your system.]))
AC_CHECK_HEADER(inttypes.h, , AC_ERROR([Could not find inttypes.h on your system.]))
AC_CHECK_HEADER(libgen.h, , AC_ERROR([Could not find libgen.h on your system.]))
AC_CHECK_HEADER(locale.h, , AC_ERROR([Could not find locale.h on your system.]))
AC_CHECK_HEADER(malloc.h, , AC_ERROR([Could not find malloc.h on your system.]))
AC_CHECK_HEADER(math.h, , AC_ERROR([Could not find math.h on your system.]))
AC_CHECK_HEADER(pthread.h, , AC_ERROR([Could not find pthread.h on your system.]))
AC_CHECK_HEADER(stddef.h, , AC_ERROR([Could not find stddef.h on your system.]))
AC_CHECK_HEADER(stdint.h, , AC_ERROR([Could not find stdint.h on your system.]))
AC_CHECK_HEADER(stdio.h, , AC_ERROR([Could not find stdio.h on your system.]))
AC_CHECK_HEADER(stdlib.h, , AC_ERROR([Could not find stdlib.h on your system.]))
AC_CHECK_HEADER(string.h, , AC_ERROR([Could not find string.h on your system.]))
AC_CHECK_HEADER(endian.h, , AC_ERROR([Could not find endian.h on your system.]))
AC_CHECK_HEADER(dlfcn.h, , AC_ERROR([Could not find dlfcn.h on your system.]))
AC_CHECK_HEADER(link.h, , AC_ERROR([Could not find link.h on your system.]))
AC_CHECK_HEADER(stdarg.h, , AC_ERROR([Could not find stdarg.h on your system.]))
AC_CHECK_HEADER(signal.h, , AC_ERROR([Could not find signal.h on your system.]))
#AC_CHECK_HEADER(sys/mman.h, AC_DEFINE(HAVE_SYS_MMAN_H, 1, [sys/mman.h is present]))

AC_CHECK_FUNCS(strdup)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_VPRINTF

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.0])
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0])
PKG_CHECK_MODULES(GLADE, [libglade-2.0 >= 2.0])

CFLAGS="$CFLAGS -Wall"

AC_OUTPUT(Makefile)
