AC_INIT([mcbenchmark], [0.1.0], [mlelansky@mail.de])
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.15 gnu subdir-objects -Wall -Werror])
AC_CONFIG_FILES([Makefile])
AC_PROG_CC
PKG_CHECK_MODULES([DEPS], [libmemcached >= 0.8.0])
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)
AC_OUTPUT
