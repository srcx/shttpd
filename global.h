/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* global.h
 * globalni definice
 */

#ifndef SHTTPD_GLOBAL_H
#define SHTTPD_GLOBAL_H

#include "aconfig.h"

/* stricmp */
#undef WANT_STRICMP
#ifndef HAVE_STRICMP
#  ifdef HAVE_STRCASECMP
#    define stricmp(s1,s2)      strcasecmp(s1,s2)
#  else
#    define WANT_STRICMP
     int stricmp (const char *s1, const char *s2);
#  endif
#endif

/* strdup */
#undef WANT_STRDUP
#ifndef HAVE_STRDUP
#  define WANT_STRDUP
   char *strdup (const char *s);
#endif

#endif
