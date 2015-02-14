dnl shttpd configure macros

dnl SHTTPD_PROG_LD_S - whether $(CC) -s works
AC_DEFUN(SHTTPD_PROG_LD_S,
[AC_MSG_CHECKING(whether linker works with -s option)
shttpd_save_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -s"
AC_CACHE_VAL(shttpd_cv_prog_ld_s, [
AC_TRY_LINK(,{},shttpd_cv_prog_ld_s=yes, shttpd_cv_prog_ld_s=no)])
LDFLAGS=$shttpd_save_LDFLAGS
AC_MSG_RESULT($shttpd_cv_prog_ld_s)
])

dnl SHTTPD_HAVE_GNU_STRFTIME - whether strftime accepts %z
AC_DEFUN(SHTTPD_HAVE_GNU_STRFTIME,
[AC_MSG_CHECKING(whether strftime has GNU extensions)
AC_CACHE_VAL(shttpd_cv_have_gnu_strftime, [
AC_TRY_RUN([
#ifdef STDC_HEADERS
#  include <stdio.h>
#  include <time.h>
#endif
int main (void) {
  char buf[32]; time_t t;
  FILE *f = fopen ("confteststrftime", "w+");
  t = time (NULL);
  strftime (buf, 32, "%z", localtime (&t));
  fprintf (f, "%s", buf);
  fclose (f);
  exit (0);
}],[
shttpd_strftime_test_result=`cat confteststrftime`
if test "x$shttpd_strftime_test_result" != "x%z"; then
  shttpd_cv_have_gnu_strftime=yes
else
  shttpd_cv_have_gnu_strftime=no
fi
], shttpd_cv_have_gnu_strftime=no)])
AC_MSG_RESULT($shttpd_cv_have_gnu_strftime)
if test "x$shttpd_cv_have_gnu_strftime" = "xyes"; then
  AC_DEFINE(HAVE_GNU_STRFTIME, 1, Define if function strftime has GNU extensions)
fi
])
