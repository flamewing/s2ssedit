dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(s2ssedit, 0.2.0)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_14([],[mandatory])
AC_HEADER_STDC

# ---------------------------------------------------------------------
# Host system settings
# ---------------------------------------------------------------------
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

# ---------------------------------------------------------------------
# Warning level
# ---------------------------------------------------------------------

CHK_WARN="-Wall -Wextra -pedantic -Wcast-qual -Wwrite-strings -Wredundant-decls -Wdisabled-optimization -fcheck-new"
CHK_WARN="$CHK_WARN -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wuseless-cast"
CHK_WARN="$CHK_WARN -Wno-format-nonliteral -Wformat-security -Wshift-negative-value -Wshift-overflow -Wtrigraphs"
CHK_WARN="$CHK_WARN -Wuninitialized -fstrict-aliasing -Wduplicated-cond -Wplacement-new -Wcast-align"
CHK_WARN="$CHK_WARN -Wlogical-op -Wlogical-not-parentheses -Wunused-const-variable -Wpointer-sign"
CHK_WARN="$CHK_WARN -Wundef"
#CHK_WARN="$CHK_WARN -Wpadded -Wshadow -Wconditionally-supported"
CHK_WARN="$CHK_WARN -Wc++11-compat -Wc++14-compat -Wc++1z-compat"
# GCC attributes
#CHK_WARN="$CHK_WARN -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format"
# C++11 and newer
#CHK_WARN="$CHK_WARN -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -Wzero-as-null-pointer-constant"
# Clang warnings
CHK_WARN="$CHK_WARN -Wunused-const-variables -Wabsolute-value -Wdeprecated-register -Wmismatched-tags -Wunused-private-field"
# Warnings give errors
AC_ARG_ENABLE(warning-errors, AS_HELP_STRING([--enable-warning-errors], [Turn all warnings into errors @<:@default no@:>@]),,enable_warning_errors=no)
if test x$enable_warning_errors = xyes; then
	CHK_WARN="-Werror $CHK_WARN"
fi

# Pedantic warnings give errors
AC_ARG_ENABLE(pedantic-errors, AS_HELP_STRING([--enable-pedantic-errors], [Turn all pedantic warnings into errors @<:@default no@:>@]),,enable_pedantic_errors=no)
if test x$enable_pedantic_errors = xyes; then
	CHK_WARN="-pedantic-errors $CHK_WARN"
fi

WARNINGS=""
for cxx_flag in $CHK_WARN
do
	AX_CHECK_COMPILE_FLAG([$cxx_flag], [WARNINGS="$WARNINGS $cxx_flag"], [], [$CXXFLAGS -Werror])
done

CXXFLAGS="$CXXFLAGS $WARNINGS"

# ---------------------------------------------------------------------
# Libtool
# ---------------------------------------------------------------------

LT_INIT

# ---------------------------------------------------------------------
# Required mdcomp headers
# ---------------------------------------------------------------------

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

PKG_CHECK_MODULES(S2SSEDIT, [gdkmm-2.4 cairomm-1.0 gtkmm-2.4 sigc++-2.0 glibmm-2.4])

AC_SUBST(EXTRA_FLAGS)
AC_SUBST(ICON_FILE)

# ---------------------------------------------------------------------
# Makefiles
# ---------------------------------------------------------------------

AC_OUTPUT([
Makefile
src/Makefile
src/lib/Makefile
])

