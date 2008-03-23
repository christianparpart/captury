AC_INIT(AUTHORS)

CAPTURY_MAJOR_VERSION=0
CAPTURY_MINOR_VERSION=3
CAPTURY_MICRO_VERSION=0
CAPTURY_RELEASE_INFO="-dev"

CAPTURY_VERSION=$CAPTURY_MAJOR_VERSION.$CAPTURY_MINOR_VERSION.$CAPTURY_MICRO_VERSION$CAPTURY_RELEASE_INFO
CAPTURY_VERSION_INFO=`expr $CAPTURY_MAJOR_VERSION + $CAPTURY_MINOR_VERSION`:$CAPTURY_MICRO_VERSION:$CAPTURY_MINOR_VERSION
CAPTURY_VERSION_NUMBER=`expr $CAPTURY_MAJOR_VERSION \* 10000 + $CAPTURY_MINOR_VERSION \* 100 + $CAPTURY_MICRO_VERSION`

AC_SUBST(CAPTURY_MAJOR_VERSION)
AC_SUBST(CAPTURY_MINOR_VERSION)
AC_SUBST(CAPTURY_MICRO_VERSION)
AC_SUBST(CAPTURY_VERSION)
AC_SUBST(CAPTURY_VERSION_INFO)
AC_SUBST(CAPTURY_VERSION_NUMBER)

VERSION=${CAPTURY_VERSION}

AM_INIT_AUTOMAKE(captury, $VERSION)

AM_CONFIG_HEADER(config.h)

AC_GNU_SOURCE
AC_ISC_POSIX

AC_PROG_INSTALL
AC_LANG_CPLUSPLUS
AC_PROG_CXX
AM_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG([0.17.2])

dnl {{{ --enable-builtin-strip-soname
AC_ARG_ENABLE([builtin-strip-soname], [
  --disable-builtin-strip-soname
                          Disables compilation of strip-soname.
                          Do so only when you have to.
                          If so, run make with STRIP_SONAME=/path/to/strip-soname],
  [enable_builtin_strip_soname=${enableval}],
  [enable_builtin_strip_soname=yes]
)
AM_CONDITIONAL([BUILTIN_STRIP_SONAME], [test x$enable_builtin_strip_soname = xyes])
dnl }}}

dnl -------------------------------------------------------------------------
PKG_CHECK_MODULES([CAPTURY], [libcaptury])
PKG_CHECK_MODULES([PNG], [libpng >= 1.2])

dnl -------------------------------------------------------------------------
AC_OUTPUT([
  Makefile
  src/Makefile
  src/libGLcaptury/Makefile
  src/captury
])

dnl vim:ai:et:ts=2:nowrap
