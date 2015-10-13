# Author: Jeroen Meijer <j.j.g.meijer@utwente.nl>
#
# SYNOPSIS
#
#   ACX_LTSmin([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
#
AC_DEFUN([ACX_LTSmin], [

AC_ARG_WITH([LTSmin],
  [AS_HELP_STRING([--with-LTSmin=<prefix>],[LTSmin prefix directory])])

case "$with_LTSmin" in
  '') CHECK_DIR="/usr/local /usr /opt/local /opt/install" ;;
  no) CHECK_DIR="" ;;
   *) CHECK_DIR="$with_LTSmin" ;;
esac
  
acx_LTSmin=no
for f in $CHECK_DIR; do
    if test -f "${f}/include/ltsmin/pins.h" && test -f "${f}/include/ltsmin/dlopen-api.h" && test -f "${f}/include/ltsmin/lts-type.h" && test -f "${f}/include/ltsmin/ltsmin-standard.h"; then
        LIBLTSMIN_INCLUDE="${f}"
        AC_SUBST([LTSMIN_CFLAGS],["-I${f}/include/ltsmin"])
        AC_SUBST([LIBLTSMIN],["${f}"])
        AC_SUBST([LTSMIN_LDFLAGS],["-L${f}/lib"])
        acx_LTSmin=yes
        break
    fi
done
])

