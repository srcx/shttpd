/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* utils.c
 * pomocne fce
 */

#include "utils.h"

#include <string.h>

/* najde dany prvek v poli stringu ukoncenem NULL */
int find_str (const char **array, char *str, int ignore_case) {
  int i = 0;
  while (*array) {
    if ((ignore_case && !stricmp (*array, str)) || (!ignore_case && !strcmp (*array, str)))
      return i;
    i++; array++;
  }
  return -1;
}

/* prevede hex pismeno na int */
int hex2int (char c) {
  if ((c >= '0') && (c <= '9')) return c - '0';
  else if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
  else if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
  
  return 0;
}

/* cteni z ridkeho pole */
void *get_sarray_item (sarray_item_t *array, int key) {
  while (array->key != -1) {
    if (array->key == key) return array->data;
    array++;
  }
  return NULL;
}
