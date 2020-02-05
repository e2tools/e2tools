dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)
AC_INIT([e2tools],
        [0.0.17],
        [https://github.com/e2tools/e2tools/issues],
        [e2tools],
        [https://e2tools.github.io/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([e2tools.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([
-Wall
-Werror
1.9.6
dist-bzip2
foreign
])

dnl Ensure that these Macros are actually available at the time of
dnl running autoreconf
m4_pattern_forbid([gl_EARLY])dnl
m4_pattern_forbid([PKG_CHECK_MODULES])dnl

dnl Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED
AC_PROG_MAKE_SET

dnl Checks for libraries.
gl_INIT
PKG_CHECK_MODULES([EXT2FS], [ext2fs >= 1.27], [], [
    AC_MSG_ERROR([Sorry, but we do require libext2fs (part of e2fsprogs).])])
PKG_CHECK_MODULES([COM_ERR], [com_err], [], [
    AC_MSG_ERROR([Sorry, but we do require libcom_err (part of e2fsprogs).])])


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
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


dnl ====================================================================
dnl The end of configure.ac is approaching fast.
dnl ====================================================================

CFLAGS="$CFLAGS -Wall"

AC_CONFIG_FILES([
    Makefile
    lib/Makefile
])
AC_OUTPUT
