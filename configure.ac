orig_CFLAGS="$CFLAGS"

AC_INIT([piapi], [2.0.1])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([src/piapi_config.h])
AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
dnl Supply default CFLAGS, if not specified
if test -z "$orig_CFLAGS"; then
  if test x$GCC = xyes; then
    CFLAGS="-g -O3"
  fi
fi

AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P
AC_PROG_LN_S

AC_C_RESTRICT
AC_C_INLINE

m4_include([config/piapi.m4])
m4_include([config/piapi_powerinsight.m4])

PIAPI_CHECK_POWERINSIGHT([piapi_have_powerinsight="yes"], [piapi_have_powerinsight="no"])

PIAPI_CHECK_DEBUG()

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tools/Makefile])

AC_OUTPUT
