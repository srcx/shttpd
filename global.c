/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* global.c
 * globalni definice
 */

#include "global.h"

#ifdef WANT_STRICMP
#include <ctype.h>

int stricmp (const char *s1, const char *s2) {
  int r = 0;

  while (!(r = (uchar)(tolower (*s1)) - (uchar)(tolower (*s2))) && (*s1++ && *s2++));

  return r;
}
#endif

#ifdef WANT_STRDUP
char *strdup (const char *s) {
  char *ns = NULL;

  if ((ns = malloc (strlen (s) + 1)))
    strcpy (ns, s);

  return ns;
}
#endif
