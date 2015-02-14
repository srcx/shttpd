/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* utils.h
 * pomocne fce
 */

#ifndef SHTTPD_UTILS_H
#define SHTTPD_UTILS_H

#include "global.h"

/* ridke pole - konci polozkou s key == -1 */
typedef struct {
  int key;				/* klic (index) */
  void *data;			/* lib. data */
} sarray_item_t;

/* najde dany prvek v poli stringu ukoncenem NULL a vrati jeho index nebo -1 */
int find_str (const char **array, char *str, int ignore_case);

/* cteni z ridkeho pole */
void *get_sarray_item (sarray_item_t *array, int key);

/* prevede hex pismeno na int */
int hex2int (char c);

#endif