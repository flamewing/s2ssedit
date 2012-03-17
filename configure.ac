dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(s2ssedit, 0.2.0)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
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

AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

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
GETTEXT_PACKAGE=s2ssedit
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])

AM_PROG_LIBTOOL

PKG_CHECK_MODULES(S2SSEDIT, [gtkmm-2.4 >= 2.12    ])

AC_SUBST(EXTRA_FLAGS)
AC_SUBST(ICON_FILE)

AC_OUTPUT([
Makefile
src/Makefile
src/lib/Makefile
po/Makefile.in
])
