/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* config.h
 * zpracovani konfiguracnich souboru
 */

#ifndef SHTTPD_CONFIG_H
#define SHTTPD_CONFIG_H

#include "global.h"

#include "utils.h"

typedef struct {
  int min_threads;				/* min. pocet bezicich vlaken */
  int max_threads;				/* max. pocet bezicich vlaken */
  int port;						/* port pro cekani na spojeni */
  char *access_log;				/* jmeno access_log souboru */
  char *error_log;				/* jmeno error_log souboru */
  int log_level;				/* min. log level pro error_log */
  int error_log_stderr;			/* kopirovat error log na stderr ? */
  char *doc_root;				/* koren dokumentu */
  int remote_names;				/* zda-li se maji prekladat IP adresy na jmena */
  sarray_item_t *status_page;	/* pole odpovedi pri chybovych stavech */
  char *index_file;				/* jmeno defaultniho index souboru */
  char *media_type;				/* defaultni media (MIME) typ */
  char *media_config;			/* soubor s konfiguraci media typu */
} server_config_t;
extern server_config_t server_config;

/* nacteni konfigurace */
int load_config (char *fname);

/* zruseni konfigurace */
void done_config (void);

#endif