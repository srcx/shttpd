/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* mime.h
 * prirazovani MIME (media) typu pro soubory
 */

#ifndef SHTTPD_MIME_H
#define SHTTPD_MIME_H

#include "global.h"

/* inicializace MIME typu */
int init_mime (const char *fname);

/* zruseni MIME typu */
void done_mime (void);

/* nalezeni MIME typu pro dany soubor */
const char *find_mime (const char *fname);

#endif
