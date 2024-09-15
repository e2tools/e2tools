dnl Add compiler flags to CFLAGS if and only if they are supported
dnl by the compiler. What is done with the augmented CFLAGS variable
dnl is up to the caller.
dnl
dnl Example:
dnl   E2T_CONDITIONAL_CFLAGS([-Wall])
dnl   E2T_CONDITIONAL_CFLAGS([-Wextra])
dnl   E2T_CONDITIONAL_CFLAGS([-Werror])
dnl   E2T_CONDITIONAL_CFLAGS([-fno-common])
dnl
AC_DEFUN([E2T_CONDITIONAL_CFLAGS], [dnl
orig_CFLAGS="$CFLAGS"
CFLAGS="$orig_CFLAGS -Werror $1"
AC_MSG_CHECKING([whether to add $1 to CFLAGS])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[char x[16];]])], [dnl
AC_MSG_RESULT([yes])
CFLAGS="$orig_CFLAGS $1"
], [dnl
AC_MSG_RESULT([no])
CFLAGS="$orig_CFLAGS"
])dnl
])dnl
dnl
dnl
dnl ====================================================================
dnl
dnl Local Variables:
dnl mode: autoconf
dnl End:
