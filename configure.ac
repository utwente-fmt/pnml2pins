AC_PREREQ([2.69])
AC_INIT([pnml2pins], [1.0], [ltsmin-support@lists.utwente.nl])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([tools])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
LT_INIT

AM_PATH_PYTHON([3])

AC_CANONICAL_HOST
AM_CONDITIONAL([DARWIN], [case $host_os in darwin*) true;; *) false;; esac])

ACX_LTSmin([:],
  [AC_MSG_WARN([No acceptable LTSmin installation found.])])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
