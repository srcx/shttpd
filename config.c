/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* config.c
 * zpracovani konfiguracnich souboru
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "logfile.h"
#include "http.h"
#include "mime.h"

static sarray_item_t status_page[] = {
  { 301, "<HTML>\n\
<HEAD><TITLE>301 Moved Permanently</TITLE></HEAD><BODY>\n\
<H1>Moved Permanently</H1>\n\
The document has moved <A HREF=\"%s\">here</A>.\n\
<HR><P><I>" SHTTPD_VERSION_STR "</I></BODY></HTML>\n", },
  { 400, "<HTML>\n\
<HEAD><TITLE>400 Bad Request</TITLE></HEAD><BODY>\n\
<H1>Bad Request</H1>\n\
Your browser send a request that this server could not understand.\n\
<HR><P><I>" SHTTPD_VERSION_STR "</I></BODY></HTML>\n", },
  { 403, "<HTML>\n\
<HEAD><TITLE>403 Forbidden</TITLE></HEAD><BODY>\n\
<H1>Forbidden</H1>\n\
You don't have permission to access %s on this server.\n\
<HR><P><I>" SHTTPD_VERSION_STR "</I></BODY></HTML>\n", },
  { 404, "<HTML>\n\
<HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY>\n\
<H1>Not Found</H1>\n\
%s was not found on this server.\n\
<HR><P><I>" SHTTPD_VERSION_STR "</I></BODY></HTML>\n", },
  { 405, "<HTML>\n\
<HEAD><TITLE>405 Method Not Allowed</TITLE></HEAD><BODY>\n\
<H1>Method Not Allowed</H1>\n\
The requested method is not allowed for URL %s.\n\
<HR><P><I>" SHTTPD_VERSION_STR "</I></BODY></HTML>\n", },
  { 505, "<HTML>\n\
<HEAD><TITLE>505 HTTP Version not supported</TITLE></HEAD><BODY>\n\
<H1>HTTP Version not supported</H1>\n\
Supported versions: HTTP/0.9, HTTP/1.0, HTTP/1.1 (partially).\n\
<HR><P><I>" SHTTPD_VERSION_STR "</I></BODY></HTML>\n", },
  { -1, NULL }
};

/* defaultni konfigurace */
server_config_t server_config = {
  /* min_threads */			4,
  /* max_threads */			16,
  /* port */				8080,
  /* access_log */			"access_log.shttpd",
  /* error_log */			"error_log.shttpd",
  /* log_level */			LOG_LEVEL_INFO,
  /* error_log_stderr */	1,
  /* doc_root */			"html/",
  /* remote_names */		1,
  /* status_page */			status_page,
  /* index_file */			"index.html",
  /* media_type */			"text/plain",
  /* media_config */		"mime.types",
};

static const char *log_level_str[] = {
  "LOG_LEVEL_INFO", "LOG_LEVEL_WARN", "LOG_LEVEL_ERR", "LOG_LEVEL_SYS", "LOG_LEVEL_MAX", NULL
};

/* precteni cisla */
int read_int (char **buf) {
  return strtol (*buf, buf, 0);
}

/* precteni retezce */
char *read_str (char **buf) {
  while (isspace ((int)(**buf))) (*buf)++;
  if (**buf == '"') {
    char *x, *t; int c;
    
    (*buf)++; x = t = *buf;
    while ((**buf != '"') && (**buf != '\n') && (**buf != 0)) {
      c = **buf;
      if (**buf == '\\') {
        switch (*(*buf+1)) {
          case '\\' : c = '\\'; break;
          case 'n'  : c = '\n'; break;
          case 't'  : c = '\t'; break;
          case 'r'  : c = '\r'; break;
          case '"'  : c = '"'; break;
        }
        (*buf)++;
      }
      *t = c; t++;
      (*buf)++;
    }
    *t = 0; (*buf)++;
    return strdup (x);
  }
  return NULL;
}

/* precteni identifikatoru */
int read_id (char **buf, const char **array) {
  char *x; int ret;
  while (isspace ((int)(**buf))) (*buf)++;
  x = strpbrk (*buf, " \t\n\r\0");
  *x = 0;
  ret = find_str (array, *buf, 0);
  *buf = x+1;
  return ret;
}

/* nacteni konfigurace */
int load_config (char *fname) {
  FILE *f; char buf[256];
 
  /* nacteni cfg souboru */
  f = fopen (fname, "r");
  if (!f) return -1;

  /* format :
   * # poznamka
   * promenna cislo [# poznamka]
   * promenna "retezec vcetne \\,\",\n,\r,\t" [# poznamka]
   * promenna identifikator [# poznamka]
   * + kombinace
   */
  while (fgets (buf, 255, f)) {
    char *x = buf, *x2;
    while (isspace ((int)(*x))) x++;
    if ((*x == '#') || !*x) continue;

    if ((x2 = strpbrk (x, " \t"))) {
      *x2 = 0; x2++; while (isspace ((int)(*x2))) x2++;
      if (!strcmp (x, 			"min_threads")) {
        if ((server_config.min_threads = read_int (&x2)) < 0) return -2;
      } else if (!strcmp (x,	"max_threads")) {
        if ((server_config.max_threads = read_int (&x2)) < 0) return -2;
      } else if (!strcmp (x,	"port")) {
        if ((server_config.port = read_int (&x2)) < 0) return -2;
      } else if (!strcmp (x,	"access_log")) {
        server_config.access_log = read_str (&x2);
      } else if (!strcmp (x,	"error_log")) {
        server_config.error_log = read_str (&x2);
      } else if (!strcmp (x,	"log_level")) {
        if ((server_config.log_level = read_id (&x2, log_level_str)) == -1) return -2;
      } else if (!strcmp (x,	"error_log_stderr")) {
        server_config.error_log_stderr = read_int (&x2);
      } else if (!strcmp (x,	"doc_root")) {
        server_config.doc_root = read_str (&x2);
      } else if (!strcmp (x,	"remote_names")) {
        server_config.remote_names = read_int (&x2);
      } else if (!strcmp (x,	"status_page")) {
        int code; char *msg;
        code = read_int (&x2);
        msg = read_str (&x2);
        /* nahrada existujiciho */
        { sarray_item_t *sa = server_config.status_page;
          while (sa->key != -1) {
            if (sa->key == code) {
              sa->data = msg;
              break;
            }
            sa++;
          }
        }
      } else if (!strcmp (x,	"index_file")) {
        server_config.index_file = read_str (&x2);
      } else if (!strcmp (x,	"media_type")) {
        server_config.media_type = read_str (&x2);
      } else if (!strcmp (x,	"media_config")) {
        server_config.media_config = read_str (&x2);
      } else return -2;
    } else return -2;
  }

  fclose (f);

  if (server_config.min_threads > server_config.max_threads) return -2;

  /* inicializace media typu */
  if (init_mime (server_config.media_config) == -1) return -1;

  return 0;
}

/* zruseni konfigurace */
void done_config (void) {
  done_mime ();
}
