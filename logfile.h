/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* logfile.h
 * vystup logu
 */

#ifndef SHTTPD_LOGFILE_H
#define SHTTPD_LOGFILE_H

#include "global.h"

#ifdef PTHREADS
#include <pthread.h>

extern pthread_mutex_t log_mutex;
#endif

typedef enum {
  ERROR_LOG,
  ACCESS_LOG,
} log_file_t;

typedef enum {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERR,
  LOG_LEVEL_SYS,
  LOG_LEVEL_MAX = 255,

  LOG_LEVEL_IGN = LOG_LEVEL_INFO,	/* ignorovani urovne pro access_log */
} log_level_t;

/* zapis do logu */
int log_printf (log_file_t log, log_level_t lev, char *format, ...);

#endif