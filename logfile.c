/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* logfile.c
 * vystup logu
 */

#include "logfile.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "config.h"

#ifdef PTHREADS
pthread_mutex_t log_mutex;
#endif

/* zapis do logu */
int log_printf (log_file_t log, log_level_t lev, char *format, ...) {
  char *fname = NULL; FILE *f; int ret;
  
  if (log == ERROR_LOG) fname = server_config.error_log;
  else if (log == ACCESS_LOG) fname = server_config.access_log;

  if (!fname) return 0;
  if ((log == ERROR_LOG) && (lev < server_config.log_level)) return 0;

  #ifdef PTHREADS
  pthread_mutex_lock (&log_mutex);
  #endif
  f = fopen (fname, "a");
  if (!f) {
    #ifdef PTHREADS
    pthread_mutex_unlock (&log_mutex);
    #endif
    return 0;	/* chyba se ignoruje */
  }
  { time_t curtime; struct tm *loctime; char buf[65];
    curtime = time (NULL);
    loctime = localtime (&curtime);
    #ifdef HAVE_GNU_STRFTIME
    strftime (buf, 64, "%d/%b/%Y:%H:%M:%S %z", loctime);
    #else
    strftime (buf, 64, "%d/%b/%Y:%H:%M:%S %Z", loctime);
    #endif
    fprintf (f, "[%s] ", buf);
    if ((log == ERROR_LOG) && server_config.error_log_stderr)
      fprintf (stderr, "[%s] ", buf);
  } 
  { va_list vl;
    va_start (vl, format);
    ret = vfprintf (f, format, vl);
    if ((log == ERROR_LOG) && server_config.error_log_stderr)
      vfprintf (stderr, format, vl);
    va_end (vl);
  }
  fclose (f);
  #ifdef PTHREADS
  pthread_mutex_unlock (&log_mutex);
  #endif

  return ret;
}
