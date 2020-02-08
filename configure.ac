dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)
AC_INIT([e2tools],
        [0.1.0],
        [https://github.com/e2tools/e2tools/issues],
        [e2tools],
        [https://e2tools.github.io/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([e2tools-autoconfig.h])
AC_CONFIG_SRCDIR([e2tools.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([
-Wall
-Werror
1.9.6
foreign
])

dnl Ensure that these Macros are actually available at the time of
dnl running autoreconf
m4_pattern_forbid([PKG_CHECK_MODULES])dnl

dnl Checks for programs.
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED
AC_PROG_MAKE_SET

AC_ARG_VAR([CHMOD], [chmod command])
AC_PATH_PROG([CHMOD], [chmod])

AC_ARG_VAR([DD], [dd command])
AC_PATH_PROG([DD], [dd])
AM_CONDITIONAL([HAVE_DD_COMMAND], [test "x$DD" != "xno"])

AC_ARG_VAR([MKE2FS], [mkfs.ext2 command])
AC_PATH_PROG([MKE2FS], [mkfs.ext2])
AM_CONDITIONAL([HAVE_MKE2FS_COMMAND], [test "x$MKE2FS" != "xno"])

dnl Checks for compiler flags
m4_pattern_forbid([E2T_])dnl
m4_pattern_forbid([E2T_CONDITIONAL_CFLAGS])dnl
AC_DEFUN([E2T_CONDITIONAL_CFLAGS], [dnl
orig_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $1"
AC_MSG_CHECKING([whether to add $1 to CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[char x[16];]])], [dnl
AC_MSG_RESULT([yes])
], [dnl
AC_MSG_RESULT([no])
CFLAGS="$orig_CFLAGS"
])dnl
])dnl

dnl Add compiler flags to CFLAGS, but only if they are actually
dnl supported by the GCC compatible compiler (probably gcc or clang).
dnl If the user does not want these flags to be used, they can run
dnl make with a CFLAGS= parameter.
AS_VAR_IF([GCC], [yes], [dnl
  E2T_CONDITIONAL_CFLAGS([--std=gnu99])
  E2T_CONDITIONAL_CFLAGS([-pedantic])
  E2T_CONDITIONAL_CFLAGS([-Wall])
  E2T_CONDITIONAL_CFLAGS([-Wextra])
  E2T_CONDITIONAL_CFLAGS([-Werror])
  E2T_CONDITIONAL_CFLAGS([-Wp,-D_FORTIFY_SOURCE=2])
  E2T_CONDITIONAL_CFLAGS([-fstack-clash-protection])
  E2T_CONDITIONAL_CFLAGS([-fcf-protection])
])

dnl Checks for libraries.
AC_DEFUN([E2T_PKG_CHECK_MODULES], [dnl
PKG_CHECK_MODULES([$1], [$2], [$3], [$4])
$1_MODVERSION=`${PKG_CONFIG} --modversion "$2" 2>/dev/null`
AC_SUBST([$1_MODVERSION])
AC_MSG_CHECKING([for $1 modversion])
AC_MSG_RESULT([[$]$1_MODVERSION])
])dnl

E2T_PKG_CHECK_MODULES([EXT2FS], [ext2fs >= 1.27], [], [
    AC_MSG_ERROR([Sorry, but we do require libext2fs (part of e2fsprogs).])])
E2T_PKG_CHECK_MODULES([COM_ERR], [com_err], [], [
    AC_MSG_ERROR([Sorry, but we do require libcom_err (part of e2fsprogs).])])


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS(gettimeofday regcomp strdup strtol)


dnl ====================================================================
dnl Check whether libext2fs implements ext2fs_xattrs_read() function
dnl ====================================================================
orig_CFLAGS="$CFLAGS"
orig_LIBS="$LIBS"
CFLAGS="$CFLAGS $EXT2FS_CFLAGS"
LIBS="$LIBS $EXT2FS_LIBS"

AC_CHECK_LIB([ext2fs], [ext2fs_xattrs_read], [dnl
have_ext2fs_xattrs=yes
], [dnl
have_ext2fs_xattrs=no
])

CFLAGS="$orig_CFLAGS"
LIBS="$orig_LIBS"

AM_CONDITIONAL([HAVE_EXT2FS_XATTRS], [test "x$have_ext2fs_xattrs" = "xyes"])
AM_COND_IF([HAVE_EXT2FS_XATTRS], [dnl
AC_DEFINE([HAVE_EXT2FS_XATTRS], [1], [Defined if libext2fs implements ext2fs_xattrs_read])
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
