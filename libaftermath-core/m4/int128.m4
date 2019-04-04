dnl @synopsis CHECK_INT128
dnl
dnl Checks if the C compiler supports 128-bit integers
dnl
dnl If both __int128 and unsigned __int128 are supported by the
dnl compiler the check passes, otherwise an error is generated.
dnl
AC_DEFUN([CHECK_INT128],[
	AC_MSG_CHECKING([whether 128-bit integers are supported by the compiler])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
				__int128 ret_i128(void) { return 0; }
				unsigned __int128 ret_u128(void) { return 0; }])],
			AC_MSG_RESULT([yes]),
			AC_ERROR([The compiler does not support 128-bit integers]))
])
