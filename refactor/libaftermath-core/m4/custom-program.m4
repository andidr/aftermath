dnl @synopsis CHECK_CUSTOM_PROG(binary)
dnl
dnl Checks for the presence of a binary using AC_PATH_PROG
dnl
AC_DEFUN([CHECK_CUSTOM_PROG], [
	PROG=$1
	PROG_UPPER=translit([[$1]], [a-z-], [A-Z_])

	AC_PATH_PROG(translit([[$1]], [a-z-], [A-Z_]), $1)

	if test "x$[]translit([[$1]], [a-z-], [A-Z_])" = x
	then
		AC_ERROR([Could not find program $PROG.])
	fi
])
