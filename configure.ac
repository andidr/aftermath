AC_INIT([aftermath], [unreleased])
AC_CONFIG_SRCDIR([src/aftermath.c])

m4_define([serial_tests], [
	m4_esyscmd([case `${AUTOMAKE:-automake} --version | head -n 1` in
		*1.11.*|*1.10.*|*1.9.*)
			;;
		*)
			echo serial-tests
			;;
	esac])
])

AM_INIT_AUTOMAKE([subdir-objects] serial_tests)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
m4_include([m4/with_check.m4])
m4_include([m4/python.m4])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_C_PROTOTYPES
AC_PROG_GREP
AC_PROG_SED
CHECK_CUSTOM_PROG(pkg-config)
CHECK_CUSTOM_PROG(mkdir)
CHECK_CUSTOM_PROG(glib-genmarshal)

LT_INIT

# Use pbzip2 if present
AC_CHECK_PROG(HAVE_PBZIP2, pbzip2, yes, no)
if test "x$HAVE_PBZIP2" = "xyes"
then
	AC_DEFINE(UNCOMPRESS_USE_PBZIP2, 1)
else
	AC_DEFINE(UNCOMPRESS_USE_PBZIP2, 0)
fi

# Checks for libraries.
CHECK_LIB_WITH(pthread, pthread, pthread_create)

# Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADER(alloca.h, , AC_ERROR([Could not find alloca.h on your system.]))
AC_CHECK_HEADER(assert.h, , AC_ERROR([Could not find assert.h on your system.]))
AC_CHECK_HEADER(ctype.h, , AC_ERROR([Could not find ctype.h on your system.]))
AC_CHECK_HEADER(endian.h, , AC_ERROR([Could not find endian.h on your system.]))
AC_CHECK_HEADER(errno.h, , AC_ERROR([Could not find errno.h on your system.]))
AC_CHECK_HEADER(fcntl.h, , AC_ERROR([Could not find fcntl.h on your system.]))
AC_CHECK_HEADER(float.h, , AC_ERROR([Could not find float.h on your system.]))
AC_CHECK_HEADER(getopt.h, , AC_ERROR([Could not find getopt.h on your system.]))
AC_CHECK_HEADER(inttypes.h, , AC_ERROR([Could not find inttypes.h on your system.]))
AC_CHECK_HEADER(libgen.h, , AC_ERROR([Could not find libgen.h on your system.]))
AC_CHECK_HEADER(limits.h, , AC_ERROR([Could not find limits.h on your system.]))
AC_CHECK_HEADER(math.h, , AC_ERROR([Could not find math.h on your system.]))
AC_CHECK_HEADER(pthread.h, , AC_ERROR([Could not find pthread.h on your system.]))
AC_CHECK_HEADER(regex.h, , AC_ERROR([Could not find regex.h on your system.]))
AC_CHECK_HEADER(search.h, , AC_ERROR([Could not find search.h on your system.]))
AC_CHECK_HEADER(signal.h, , AC_ERROR([Could not find signal.h on your system.]))
AC_CHECK_HEADER(stdarg.h, , AC_ERROR([Could not find stdarg.h on your system.]))
AC_CHECK_HEADER(stdbool.h, , AC_ERROR([Could not find stdbool.h on your system.]))
AC_CHECK_HEADER(stddef.h, , AC_ERROR([Could not find stddef.h on your system.]))
AC_CHECK_HEADER(stdint.h, , AC_ERROR([Could not find stdint.h on your system.]))
AC_CHECK_HEADER(stdio.h, , AC_ERROR([Could not find stdio.h on your system.]))
AC_CHECK_HEADER(stdlib.h, , AC_ERROR([Could not find stdlib.h on your system.]))
AC_CHECK_HEADER(string.h, , AC_ERROR([Could not find string.h on your system.]))
AC_CHECK_HEADER(sys/mman.h, , AC_ERROR([Could not find sys/mman.h on your system.]))
AC_CHECK_HEADER(sys/stat.h, , AC_ERROR([Could not find sys/stat.h on your system.]))
AC_CHECK_HEADER(sys/time.h, , AC_ERROR([Could not find sys/time.h on your system.]))
AC_CHECK_HEADER(time.h, , AC_ERROR([Could not find time.h on your system.]))
AC_CHECK_HEADER(sys/types.h, , AC_ERROR([Could not find sys/types.h on your system.]))
AC_CHECK_HEADER(sys/wait.h, , AC_ERROR([Could not find sys/wait.h on your system.]))
AC_CHECK_HEADER(unistd.h, , AC_ERROR([Could not find unistd.h on your system.]))

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

CHECK_CUSTOM_PROG(python)
AC_PYTHON_MODULE(sets, true)
AC_PYTHON_MODULE(sys, true)
AC_PYTHON_MODULE(shlex, true)
AC_PYTHON_MODULE(re, true)
AC_PYTHON_MODULE(os, true)
AC_PYTHON_MODULE(numpy, true)

CFLAGS="$CFLAGS -Wall"

CONFIG_DATE=`date +%m/%d/%Y`
AC_SUBST(CONFIG_DATE)
AC_OUTPUT([Makefile doc/man/aftermath.1])

CHECK_HEADER_CFLAGS="$DEFS $GLIB_CFLAGS $CFLAGS $GTK_CFLAGS $GLADE_CFLAGS"
AC_SUBST(CHECK_HEADER_CFLAGS)
AC_OUTPUT([scripts/check-header.sh], [chmod +x scripts/check-header.sh])

mkdir -p test-tmp
