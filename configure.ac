# Copyright 2017 Mirko Lelansky <mlelansky@mail.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
AC_INIT([mcbenchmark], [0.1.0], [mlelansky@mail.de])
AC_PREREQ([2.69])
LT_PREREQ([2.4.6])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.15 gnu subdir-objects -Wall -Werror])
LT_INIT
AC_PROG_CC
AC_PROG_CC_STDC
AC_PATH_PROG(GENGETOPT, gengetopt)
AM_CONDITIONAL([GENGETOPT_MISSING], [test "x$GENGETOPT" = "x"])
AM_COND_IF([GENGETOPT_MISSING], [AC_MSG_ERROR(The gengentop programm is missing.)])
PKG_CHECK_MODULES([MEMCACHED], [libmemcached >= 0.8.0])
AC_CHECK_HEADERS([getopt.h stdio.h])
AX_PTHREAD([], [AC_MSG_ERROR(The pthread library is missing.)])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
