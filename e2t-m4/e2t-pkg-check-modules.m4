dnl Like PKG_CHECK_MODULES, but AC_MSG_RESULTs the pkg-config modversion.
dnl
dnl Example:
dnl   E2T_PKG_CHECK_MODULES([EXT2FS], [ext2fs >= 1.27], [], [dnl
dnl     AC_MSG_ERROR([Sorry, but libext2fs (part of e2fsprogs) is required.])
dnl   ])
dnl
AC_DEFUN([E2T_PKG_CHECK_MODULES], [dnl
PKG_CHECK_MODULES([$1], [$2], [$3], [$4])
$1_MODVERSION=`${PKG_CONFIG} --modversion "$2" 2>/dev/null`
AC_SUBST([$1_MODVERSION])
AC_MSG_CHECKING([for $1 modversion])
AC_MSG_RESULT([[$]$1_MODVERSION])
])dnl
dnl
dnl
dnl ====================================================================
dnl
dnl Local Variables:
dnl mode: autoconf
dnl End:
