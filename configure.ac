dnl ====================================================================
dnl Process this file with autoconf to produce a configure script.
dnl ====================================================================
dnl
dnl Relevant build tool versions:
dnl
dnl   2003-11-06  autoconf 2.59    Original e2tools requirement
dnl   2009-07-26  autoconf 2.64    About the time of automake 1.11
dnl   2012-04-24  autoconf 2.69    Latest release for a loooong time
dnl
dnl   2005-07-10  automake 1.9.6   Original e2tools requirement
dnl   2009-05-17  automake 1.11    Adds AM_SILENT_RULES and silent-rules
dnl   2024-07-11  automake 1.17    Requires -Wno-portability for # in make vars
dnl
dnl ====================================================================
AC_PREREQ(2.59)
AC_INIT([e2tools],
        [0.1.3],
        [https://github.com/e2tools/e2tools/issues],
        [e2tools],
        [https://e2tools.github.io/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([e2tools-autoconfig.h])
AC_CONFIG_SRCDIR([src/e2tools.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([
-Wall
-Werror
-Wno-portability
1.9.6
foreign
subdir-objects
])


dnl ====================================================================
dnl Checks for programs.
dnl ====================================================================
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED
AC_PROG_MAKE_SET

AC_ARG_VAR([CHMOD], [chmod command])
AS_VAR_IF([CHMOD], [], [dnl
  AC_PATH_PROG([CHMOD], [chmod])
])
AC_MSG_CHECKING([CHMOD value to be used])
AC_MSG_RESULT([$CHMOD])

AC_ARG_VAR([DD], [dd command])
AS_VAR_IF([DD], [], [dnl
  AC_PATH_PROG([DD], [dd])
])
AC_MSG_CHECKING([DD value to be used])
AC_MSG_RESULT([$DD])
AM_CONDITIONAL([HAVE_DD_COMMAND],
               [test "x$DD" != "x" && test "x$DD" != "xno"])

AC_ARG_VAR([MKE2FS], [mke2fs or mkfs.ext2 command])
AS_VAR_IF([MKE2FS], [], [dnl
  AC_PATH_PROG([MKE2FS], [mke2fs mkfs.ext2])
])
AC_MSG_CHECKING([MKE2FS value to be used])
AC_MSG_RESULT([$MKE2FS])
AM_CONDITIONAL([HAVE_MKE2FS_COMMAND],
               [test "x$MKE2FS" != "x" && test "x$MKE2FS" != "xno"])


dnl ====================================================================
dnl Checks for compiler flags
dnl ====================================================================

dnl Add compiler flags to CFLAGS, but only if they are actually
dnl supported by the GCC compatible compiler (probably gcc or clang).
dnl If the user does not want these flags to be used, they can run
dnl make with a CFLAGS= parameter.
m4_pattern_forbid([E2T_CONDITIONAL_CFLAGS])dnl
dnl
AS_VAR_IF([GCC], [yes], [dnl
  E2T_CONDITIONAL_CFLAGS([--std=gnu99])
  E2T_CONDITIONAL_CFLAGS([-pedantic])
  E2T_CONDITIONAL_CFLAGS([-Wall])
  E2T_CONDITIONAL_CFLAGS([-Wextra])
  E2T_CONDITIONAL_CFLAGS([-Wmost])
  dnl E2T_CONDITIONAL_CFLAGS([-Werror])
  E2T_CONDITIONAL_CFLAGS([-Wp,-D_FORTIFY_SOURCE=2])
  E2T_CONDITIONAL_CFLAGS([-fstack-clash-protection])
  E2T_CONDITIONAL_CFLAGS([-fcf-protection])
])


dnl ====================================================================
dnl Checks for libraries.
dnl   E2T_PKG_CHECK_MODULES has the advantage over PKG_CHECK_MODULES
dnl   that it also thecks the modversion and AC_MSG_RESULTs it for
dnl   help with debugging issues with different library versions.
dnl ====================================================================
m4_pattern_forbid([E2T_PKG_CHECK_MODULES])dnl
dnl
E2T_PKG_CHECK_MODULES([EXT2FS], [ext2fs >= 1.27], [], [dnl
    AC_MSG_ERROR([AC_PACKAGE_NAME requires libext2fs (part of e2fsprogs).])])
E2T_PKG_CHECK_MODULES([COM_ERR], [com_err], [], [dnl
    AC_MSG_ERROR([AC_PACKAGE_NAME requires libcom_err (part of e2fsprogs).])])


dnl ====================================================================
dnl Checks for header files.
dnl ====================================================================
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h sys/time.h unistd.h])


dnl ====================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ====================================================================
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_HEADER_TIME
AC_STRUCT_TM


dnl ====================================================================
dnl Checks for library functions.
dnl ====================================================================
AC_FUNC_ALLOCA
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([gettimeofday regcomp strdup strtol])


dnl ====================================================================
dnl Check whether libext2fs implements ext2fs_xattrs_read() function
dnl ====================================================================
orig_CFLAGS="$CFLAGS"
orig_LIBS="$LIBS"
CFLAGS="$CFLAGS $EXT2FS_CFLAGS"
LIBS="$LIBS $EXT2FS_LIBS"

AC_CHECK_LIB([ext2fs], [ext2fs_xattrs_read],
             [have_ext2fs_xattrs=yes], [have_ext2fs_xattrs=no])

CFLAGS="$orig_CFLAGS"
LIBS="$orig_LIBS"

AM_CONDITIONAL([HAVE_EXT2FS_XATTRS], [test "x$have_ext2fs_xattrs" = "xyes"])
AM_COND_IF([HAVE_EXT2FS_XATTRS], [dnl
  AC_DEFINE([HAVE_EXT2FS_XATTRS], [1],
            [Defined if libext2fs implements ext2fs_xattrs_read])
])


dnl ====================================================================
dnl Check whether we can compile a function with __attribute__((naked))
dnl ====================================================================
AC_MSG_CHECKING([whether __attribute__((naked)) is available])
orig_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall -Wextra -pedantic -Werror"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
static void test_function(void)
  __attribute__(( naked ))
  __attribute__(( used ));
static void test_function(void)
{
  switch (0) {
  case 0:
    break;
  }
}
]])], [dnl
  AC_MSG_RESULT([yes])
  AC_DEFINE([FUNCTION_ATTRIBUTE_NAKED_IF_POSSIBLE], [__attribute__((naked))],
            [defined properly when compiling with naked attribute works])
], [dnl
  AC_MSG_RESULT([no])
  AC_DEFINE([FUNCTION_ATTRIBUTE_NAKED_IF_POSSIBLE], [],
            [defined empty when compiling with naked attribute fails])
])
CFLAGS="$orig_CFLAGS"


dnl ====================================================================
dnl The end of configure.ac is approaching fast.
dnl ====================================================================
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
