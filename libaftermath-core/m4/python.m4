dnl Taken from http://ac-archive.sourceforge.net/ac-archive/ac_python_module.html
dnl
dnl The Copyright notice on the above-mentioned web page states;
dnl
dnl   Author: Andrew Collier <colliera@nu.ac.za>
dnl
dnl   Copying and distribution of this file, with or without modification,
dnl   are permitted in any medium without royalty provided the copyright
dnl   notice and this notice are preserved. Users of this software should
dnl   generally follow the principles of the MIT License includings its
dnl   disclaimer.

dnl @synopsis AC_PYTHON_MODULE(modname[, fatal])
dnl
dnl Checks for Python module.
dnl
dnl If fatal is non-empty then absence of a module will trigger an
dnl error.
dnl
dnl @category InstalledPackages
dnl @author Andrew Collier <colliera@nu.ac.za>.
dnl @version 2004-07-14
AC_DEFUN([AC_PYTHON_MODULE],[
	AC_MSG_CHECKING(python module: $1)
	python -c "import $1" 2> /dev/null
	if test $? -eq 0;
	then
		AC_MSG_RESULT(yes)
		eval AS_TR_CPP(HAVE_PYMOD_$1)=yes
	else
		AC_MSG_RESULT(no)
		eval AS_TR_CPP(HAVE_PYMOD_$1)=no
		#
		if test -n "$2"
		then
			AC_MSG_ERROR(failed to find required module $1)
			exit 1
		fi
	fi
])
