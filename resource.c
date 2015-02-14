/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* resource.c
 * nacitani zdroju
 */

#include "resource.h"

#include <string.h>
#include <errno.h>
#include "config.h"

/* najde dany zdroj
 * - vraci plne jmeno souboru nebo NULL pri chybe
 * - nastavuje errno - navic EBADMSG -> uri neni absolutni
 *                           EISDIR  -> uri je adresar, ale nekonci na '/'
 */
char *find_resource (request_uri_t *uri, char *buf, int maxlen, struct stat *st) {
  char *buf_old = buf; int end_slash = 1;

  if (!uri->path) return NULL;

  strncpy (buf, server_config.doc_root, maxlen);
  buf = strchr (buf, 0);

  { char *x = uri->path;

    if (*x != '/') { errno = EBADMSG; return NULL; }
    x++;
    while (*x) {
      if (end_slash && (*x == '.')) {	/* zruseni '/.' sekvenci */
        x++;
      } else {
        if (buf - buf_old < maxlen - 1) {
          *buf = *x;
          end_slash = (*buf == '/');
          buf++;
        }
      }
      x++;
    }
  }
  *buf = 0;
  if (end_slash && server_config.index_file)
    strncat (buf_old, server_config.index_file, maxlen);

  if (stat (buf_old, st) == -1) return NULL;

  if (S_ISDIR (st->st_mode) && !end_slash) { errno = EISDIR; return NULL; }

  return buf;
}