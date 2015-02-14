/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* mime.c
 * prirazovani MIME (media) typu pro soubory
 */

#include "mime.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"

typedef struct mime_type_ext_t {
  char *ext;						/* pripona */
  char *type;						/* MIME typ */
  int free_type;					/* ma se zrusit type ? */
  struct mime_type_ext_t *next;		/* dalsi v rade */
} mime_type_ext_t;

static mime_type_ext_t *mime_types = NULL;

/* inicializace MIME typu */
int init_mime (const char *fname) {
  FILE *f; char buf[256];

  if (!fname) return 0;
  f = fopen (fname, "r");
  if (!f) return -1;

  /* format :
   * # poznamka
   * MIME_typ seznam_koncovek_souboru
   */

  while (fgets (buf, 255, f)) {
    char *x = buf, *x2; mime_type_ext_t *m;
    while (isspace ((int)*x)) x++;
    if ((*x == '#') || !*x) continue;

    if ((x2 = strpbrk (x, " \t"))) {
      char *type = NULL;
      /* cteni jednoho typu */
      *x2 = 0; x2++;
      while (isspace ((int)*x2)) x2++;
      while (*x2) {
        m = malloc (sizeof (mime_type_ext_t));
        if (!m) return -1;
        if (!type) {
          m->type = type = strdup (x);
          m->free_type = 1;
        } else {
          m->type = type;
          m->free_type = 0;
        }
        { char *xx = strpbrk (x2, " \t\n\0");
          *xx = 0;
          m->ext = strdup (x2);
          x2 = xx + 1;
        }
        m->next = mime_types;
        mime_types = m;
        while (isspace ((int)*x2)) x2++;
      }
    }
  }

  fclose (f);
  
  return 0;
}

/* zruseni MIME typu */
void done_mime (void) {
  while (mime_types) {
    mime_type_ext_t *m;
    free (mime_types->ext);
    if (mime_types->free_type) free (mime_types->type);
    m = mime_types->next;
    free (mime_types);
    mime_types = m;
  }
}

/* nalezeni MIME typu pro dany soubor */
const char *find_mime (const char *fname) {
  mime_type_ext_t *m = mime_types; const char *ext;

  ext = strrchr (fname, '.');
  if (ext) {
    ext++;
    while (m) {
      if (!stricmp (ext, m->ext)) return m->type;
      m = m->next;
    }
  }

  return server_config.media_type;
}
