dnl AC_FIND_PATH(VARIABLE, FILE-TO-FIND, VALUE-IF-NOT-FOUND, SPACE-SEPERATED-PATH)
dnl Look for FILE-TO-FIND in SPACE-SEPERATED-PATH, and return the path to
dnl the file if found, otherwise return VALUE-IF-NOT-FOUND
AC_DEFUN(AC_FIND_PATH,[
AC_MSG_CHECKING(for $2)
AC_CACHE_VAL(ac_cv_$1,ac_cv_$1="")
INPATH=$ac_cv_$1
if test "$INPATH" = ""; then
 for i in $4; do
   if test -f "$i/$2"; then
   INPATH=$i
   fi
 done
fi

if test "$INPATH" = ""; then
AC_MSG_RESULT(no)
$1=$3
else
AC_MSG_RESULT($INPATH)
$1=$INPATH
fi
AC_SUBST($1)dnl
ac_cv_$1=$$1
])
