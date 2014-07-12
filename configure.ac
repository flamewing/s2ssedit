dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(s2ssedit, 0.2.0)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE

dnl ***************************************************************************
dnl Host system settings
dnl ***************************************************************************
EXTRA_FLAGS=""
ICON_FILE=""
# determine windowing system from 'host'
AC_MSG_CHECKING([windowing system])
case "$host_os" in
	linux*)
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT([X11 (GNU/Linux)])
		;;
	mingw32* )
		WINDOWING_SYSTEM="-DWIN32"
		ICON_FILE="s2sseditico.o"
		EXTRA_FLAGS=" -lmingw32 -mwindows "
		AC_MSG_RESULT([Win32 (mingw32)])
		;;
	*)
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT([system still unsupported... guessing X11])
		;;
esac

AM_CONDITIONAL(BUILD_ICON, test ! x$ICON_FILE = x)

AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([],[mandatory])
AC_HEADER_STDC

CXXFLAGS="$CXXFLAGS -Wall -Wextra -pedantic -Wcast-qual -Wold-style-cast"
CXXFLAGS="$CXXFLAGS -Wwrite-strings -Wredundant-decls -fcheck-new"
CXXFLAGS="$CXXFLAGS -Wstrict-overflow -Wdisabled-optimization -Wctor-dtor-privacy"
CXXFLAGS="$CXXFLAGS -Wnon-virtual-dtor -Woverloaded-virtual"

AC_CHECK_HEADER(mdcomp/kosinski.h,break,[HAVE_KOSINSKI="no"])
if test "$HAVE_KOSINSKI" = "no"; then
    echo "Fatal error: Kosinski header not found!"
    exit 1
fi
AC_CHECK_HEADER(mdcomp/nemesis.h,break,[HAVE_NEMESIS="no"])
if test "$HAVE_NEMESIS" = "no"; then
    echo "Fatal error: Nemesis header not found!"
    exit 1
fi

dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=s2ssedit
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT

LT_INIT

PKG_CHECK_MODULES(S2SSEDIT, [gdkmm-2.4 cairomm-1.0 gtkmm-2.4 sigc++-2.0 glibmm-2.4])

AC_SUBST(EXTRA_FLAGS)
AC_SUBST(ICON_FILE)

AC_OUTPUT([
Makefile
src/Makefile
src/lib/Makefile
po/Makefile.in
])
