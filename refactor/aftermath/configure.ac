AC_PREREQ([2.69])
AC_INIT([aftermath], [0.2])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIRS([m4])

m4_include([m4/with-check.m4])

AC_PROG_CXX

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(QT, [Qt5Core, Qt5Gui, Qt5Widgets], [],
	[AC_MSG_ERROR([Could not find required QT libraries.])])

CPPFLAGS="`$PKG_CONFIG --cflags-only-I Qt5Core Qt5Gui Qt5Widgets` $CPPFLAGS -fPIC"
LDFLAGS="`$PKG_CONFIG --libs-only-L Qt5Core Qt5Gui Qt5Widgets` $LDFLAGS -fPIC"
LIBS="`$PKG_CONFIG --libs-only-l Qt5Core Qt5Gui Qt5Widgets` $LIBS"

AC_CHECK_PROGS(MOC, [moc-qt5 moc])
AC_CHECK_PROGS(UIC, [uic-qt5 uic])
AC_CHECK_PROGS(RCC, [rcc])

if test -z "$MOC" || test -z "$UIC" || test -z "$RCC"; then
	AC_MSG_ERROR([Qt utility programs moc, uic, and rcc are required.])
fi

CHECK_LIB_AND_HEADER_WITH([aftermath-core], [aftermath-core],
	[aftermath/core/base_types.h], [am_dsk_load_trace])

PKG_CHECK_MODULES(CAIRO, [cairo >= 1.0])
CFLAGS="$CFLAGS $CAIRO_CFLAGS"
CPPFLAGS="$CPPFLAGS $CAIRO_CFLAGS"
LIBS="$LIBS $CAIRO_LIBS"

CHECK_LIB_AND_HEADER_WITH([aftermath-render], [aftermath-render],
	[aftermath/render/timeline/renderer.h], [am_timeline_renderer_init])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
